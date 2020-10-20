#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <cgroup.h>
#include <time.h>
#include <signal.h>

#define STACK_SIZE(1024*1024)
#define MEMORY_LIMIT(512*1024*1024)

const char* rootfs = "/data/centos6/rootfs"
const char* hostname = "mydocker"

static char child_stack[STACK_SIZE];
char* const child_args[]={
	"/bin/sh",
	NULL
};

int pipe_fd[2];
int child_main(void* args){
	char c;
	printf("In child process(container)\n");
	chroot(rootfs)
	if (errno!=0){
		perror("chroot()");
		exit(1);
	}

	sethostname(hostname,sizeof(hostname));
	if (errno!=0){
		perror("sethostname()");
		exit(1);
	}

	mount("proc","/proc","proc",0,NULL);
	if(errno!=0){
		perror("Mount(proc)");
		exit(1);
	}

	chdir("/");

	close(pipe_fd[1]);
	read(pipe_fd[0],&c,1);
	system("ip link set lo up");
	system("ip link veth1 up");
	system("ip addr add 192.168.1.106/30 dev veth1");

	execv(child_args[0],child_args);
	return 1;
}

struct cgroup* cgroup_control(pid_t pid){
	struct cgroup *cgroup=NULL;
	int ret;
	ret=cgroup_init();
	char* cgname=malloc(19*sizeof(char));
	if(ret){
		printf("error occurs while init cgroup.\n");
		return NULL;
	}

	time_t now_time=time(NULL);
	sprintf(cgname,"mydocker_%d",(int)now_time);
	printf("%s\n",cgname);
	cgroup = cgroup_new_cgroup(cgname);
	if(!cgroup){
		ret = ECGFAIL;
		printf("Error new cgroup%s\n",cgroup_strerror(ret));
		goto out;
	}

	struct cgroup_controller *cgc= cgroup_add_controller(cgroup,"memory");
	struct cgroup_controller *cgc_cpuset=cgroup_add_controller(cgroup,"cpuset");

	if(!cgc||!cgc_cpuset){
		ret=ECGINVAL;
		printf("Error add controller %s\n",cgroup_strerror(ret));
		goto out;
	}

	if(cgroup_add_value_uint64(cgc,"memory.limit_in_bytes",MEMORY_LIMIT)){
		printf("Error limit memory.\n");
		goto out;
	}

	if(cgroup_add_value_string(cgc_cpuset,"cpuset.cpus","0-1")){
		printf("Error limit cpuset cpus.\n");
		goto out;
	}

	if(cgroup_add_value_string(cpc_cpuset,"cpuset.mems","0-1")){
		printf("Error limit cpuset mems.\n");
		goto out;
	}

	ret = cgroup_create_cgroup(cgroup,0);

	if(ret){
		printf("Error create cgroup%s\n",cgroup_strerror(ret));
		goto out;
	}
	ret=cgroup_attach_task_pid(cgroup,pid);
	if(ret){
		printf("Error attach_task_pid%s\n",cgroup_strerror(ret));
		goto out;
	}

	return cgroup;

out:
	if(cgroup){
		cgroup_delete_cgroup(cgroup,0);
		cgroup_free(&cgroup);
	}
	return NULL;
}

int main(){
	char* cmd; 
	printf("main process:\n");

	pipe(pipe_fd);
	if(errno!=0){
		perror("pipe()");
		exit(1);
	}

	int child_pid=clone(child_main,child_stack+STACK_SIZE,CLONE_NEWNET|CLONE_NEWNS|CLONE_NEWPID|CLONE_NEWIPC|CLONE_NEWTS|SIGCHLD,NULL);

	struct cgroup* cg=cgroup_control(child_pid);

	system("ip link add veth0 type veth peer name veth1");
	asprintf(&cmd,"ip link set veth1 netns %d",child_pid);
	system(cmd);
	system("ip link set veth0 up");
	system("ip addr add 192.168.1.106/30 dev veth0");
	free(cmd);

	close(pipe_fd[1]);
	waitpid(child_pid,NULL,0);
	if(cg){
		cgroup_delete_cgroup(cg,0);
	}
	printf("child process exited.\n");
	return 0;
}

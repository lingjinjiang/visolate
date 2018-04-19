#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <stdio.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#define STACK_SIZE (1024 * 1024)
static char container_stack[STACK_SIZE];

char* const container_args[] = {
	"/bin/bash",
	"-l",
	NULL
};

typedef struct {
	char* name;
	char* rootfs;
	char* image;
} container_info;

container_info pharse_pram(int argv, char** args) {
	container_info info;
	if (argv >= 4) {
		info.name = args[1];
		info.rootfs = args[2];
		info.image = args[3];
	}

	return info;
}

int container_main(container_info* info) {
	char* container_name = info->name;
	char* container_rootfs = info->rootfs;
	char* container_image = info->image;

	char proc_dir[strlen(container_rootfs)];
	char root_dir[strlen(container_rootfs)];

	strcpy(proc_dir, container_rootfs);
	strcpy(root_dir, container_rootfs);

	printf("Enter Container, pid: %s\n", getpid());

	sethostname(container_name, 10);

	if (mount(container_image, container_rootfs, "ext3", MS_BIND, NULL) != 0) {
		perror("rootfs");
	}

	if (mount("proc", strcat(proc_dir, "/proc"), "proc", 0, NULL) != 0) {
		perror("proc");
	}

	execv(container_args[0], container_args);
	perror("Failed to create container.");
	return 1;
}

int create_container(container_info info) {
	int pid = -1;
	pid = clone(container_main,
				container_stack + STACK_SIZE,
				CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD,
				&info);
	return pid;
}

int modify_cgroup(int pid) {
	waitpid(pid, NULL, 0);
	return 0;
}

int main(int argv, char** args) {
	container_info info = pharse_pram(argv, args);
	int pid = create_container(info);
	modify_cgroup(pid);
	return 0;
}
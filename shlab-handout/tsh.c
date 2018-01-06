/* 
 * tsh - A tiny shell program with job control
 * 
 * <Put your name and login ID here>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Misc manifest constants */
#define MAXLINE    1024   /* max line size */
#define MAXARGS     128   /* max args on a command line */
#define MAXJOBS      16   /* max jobs at any point in time */
#define MAXJID    1<<16   /* max job ID */

/* Job states */
#define UNDEF 0 /* undefined */
#define FG 1    /* running in foreground */
#define BG 2    /* running in background */
#define ST 3    /* stopped */

/* 
 * Jobs states: FG (foreground), BG (background), ST (stopped)
 * Job state transitions and enabling actions:
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 */

/* Global variables */
extern char **environ;      /* defined in libc 环境变量(字符指针数组)*/
char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
int verbose = 0;            /* if true, print additional output */
int nextjid = 1;            /* next job ID to allocate */
char sbuf[MAXLINE];         /* for composing sprintf messages */

struct job_t {              /* The job struct */
    pid_t pid;              /* job PID */
    int jid;                /* job ID [1, 2, ...] */
    int state;              /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE];  /* command line */
};
struct job_t jobs[MAXJOBS]; /* The job list */
/* End global variables */


/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
int parseline(const char *cmdline, char **argv); 
void sigquit_handler(int sig);

void clearjob(struct job_t *job);
void initjobs(struct job_t *jobs);
int maxjid(struct job_t *jobs); 
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *jobs, pid_t pid); 
pid_t fgpid(struct job_t *jobs);
struct job_t *getjobpid(struct job_t *jobs, pid_t pid);
struct job_t *getjobjid(struct job_t *jobs, int jid); 
int pid2jid(pid_t pid); 
void listjobs(struct job_t *jobs);

void usage(void);
void unix_error(char *msg);
void app_error(char *msg);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

/*
 * main - The shell's main routine 
 */
int main(int argc, char **argv) 
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);//复制文件描述符

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
	    break;
        case 'v':             /* emit additional diagnostic info */
            verbose = 1;
	    break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
	    break;
	default:
            usage();
	}
    }

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */
    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler); 

    /* Initialize the job list */
    initjobs(jobs);

    /* Execute the shell's read/eval loop */
    while (1) 
    {
        /* Read command line */
        if (emit_prompt) {
            printf("%s", prompt);//默认printf是缓冲输出的
            fflush(stdout);  //使stdout清空，把输出缓冲区里的东西打印到标准输出设备上
        }
        if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
            app_error("fgets error");
        if (feof(stdin)) { /* End of file (ctrl-d) */
            fflush(stdout);
            exit(0);
        }
        /* Evaluate the command line */
        eval(cmdline);
        fflush(stdout);    //fflush(stdin)把输入缓冲区里的东西丢弃
        fflush(stdout);
    } 
    exit(0); /* control never reaches here */
}
  
/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.  
 *
 *  pipes (|) or I/O redirection (<and>)
*/
void eval(char *cmdline) 
{
    /*
    1.参数解析(parseline)
    2.判断是内置命令(builtin_cmd),还是新建的addjob
    */
    pid_t pid;  /* warning: ‘pid’ may be used uninitialized in this function */
    char *argv[MAXARGS];
    char buf[MAXLINE];
    int bg;
    sigset_t mask_all,prev_all,mask_one,prev_one;
    strcmp(buf,cmdline);
    bg = parseline(buf,argv); 
    if(argv[0] == NULL)
        return;
    
    sigfillset(&mask_all);
    sigemptyset(&mask_one);
    sigaddset(&mask_one,SIGCHLD);

    if(!builtin_cmd(argv))
    {
        sigprocmask(SIG_BLOCK,&mask_one,&prev_one);
        if((pid = fork()) < 0) unix_error("eval: fork error");
        if(pid == 0)
        {
            sigprocmask(SIG_SETMASK,&prev_one,NULL);//子进程unblock
            if(setpgid(0, 0) < 0)  /*将当前进程的进程号设置为所在组的进程组的组号*/
                unix_error("eval: setgpid failed.\n");
            if(execve(argv[0],argv,environ)<0)
            {
                printf("%s:Command not found.\n",argv[0]); 
                exit(0);
            }    
        }
    }
    sigprocmask(SIG_BLOCK,&mask_all,&prev_all); 
    if(!(addjob(jobs, pid, bg+1, cmdline))) //(bg == 1 ? BG : FG)
         unix_error("eval: addjob error");
    sigprocmask(SIG_SETMASK,&prev_one,NULL);

    /*parent waits for foreground job to terminate
    当子进程结束时发出SIGCHLD信号后，由sigchld_handler（）处理并回收僵尸进程并从jobs中删除该前台进程。
    我们在程序中运用sleep函数来等待jobs列表中是否还存在前台进程，如果不存在则返回*/
   if(!bg)
        waitfg(pid);//不要用waitpid(pid,NULL,0);在处理程序用到了waitpid,这样在两个地方都会回收僵死子进程
    else
        printf("%d %s",pid,cmdline);  

    return ;
}

/* 
 * parseline - Parse the command line and build the argv array.
 *
 * command [arguments...] [< infile] [> oufile] [&] 
 * 如:bin/ls -l -d    ./myspin 10
 * pipes (|) or I/O redirection (<and>)
 * 
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.  
 */
int parseline(const char *cmdline, char **argv) 
{
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    int bg;                     /* background job? */
    strcpy(buf, cmdline);
    buf[strlen(buf)-1] = ' ';  /* replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* ignore leading spaces */
	    buf++;
    /* Build the argv list */
    argc = 0;
    if (*buf == '\'') //转义字符,代表一个单引号（'）字符
    {
	    buf++;
	    delim = strchr(buf, '\'');
    }
    else 
    {
	    delim = strchr(buf, ' '); //查找字符串s中首次出现字符c的位置
    }
    while (delim) 
    {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) /* ignore spaces */
            buf++; 
        if (*buf == '\'') 
        {
            buf++;
            delim = strchr(buf, '\'');
        }
        else 
        {
            delim = strchr(buf, ' ');
        }
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* ignore blank line */
	    return 1;

    /* should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0) {
	    argv[--argc] = NULL;
    }
    /*
    int k=0;
    for(k=0;k<argc;k++)
        printf("%s is the %d argument.\n",argv[k],k+1);
    */
    return bg;
}

/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.  
 */
int builtin_cmd(char **argv) 
{
    //(bg <job> fg <job>  kill <job>  jobs quit)
    if(!strcasecmp(argv[0],"quit"))
        exit(0);  //退出父进程
    if(!strcmp("&",argv[0]))
        return 1;
    if(strcmp("jobs",argv[0])==0)
    {
        listjobs(jobs);
        return 1;
    }
    if(strcmp("bg",argv[0])==0 || strcmp("fg",argv[0])==0)
    {
        do_bgfg(argv);
        return 1;
    }
    return 0;     /* not a builtin command */
}

/* 
 * do_bgfg - Execute the builtin bg and fg commands
 * 
 * Jobs states: FG (foreground), BG (background), ST (stopped)
 * Job state transitions and enabling actions:
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 */
void do_bgfg(char **argv) //对已存在的job操作
{
    int jid,pid;
    struct job_t * job;
    if(argv[1] == NULL)
    {
        printf("%s command requires PID or [jobid] argument.\n", argv[0]);
        return ;
    }
    else
    {
        //pid
        if(sscanf(argv[1],"%d",&pid) == 1)
        {
            if(!(job = getjobpid(jobs,pid)))
            {
                //jid = pid2jid(pid);
                printf("%d:No such job\n",pid);
                return ;
            }
            else
            {
                 if(strcmp("bg",argv[0])==0) 
                {
                    //pid<-1时，信号将送往以-pid为组标识的进程。
                    if(kill(-(job->pid), SIGCONT)<0) //kill为进程产生信号,而且信号不会被阻塞.
                        unix_error("do_bgfg: Kill error");
                    job->state = BG;
                        
                }
                else if(strcmp("fg",argv[0])==0)
                {
                    if(kill(-(job->pid), SIGCONT)<0)
                        unix_error("do_bgfg: Kill error");
                    job->state = FG;
                    waitfg(job->pid);////?
                }
            }
        }
        //jid
        //%%%d -- 输出一个字符%号再接着按整型输出变量的值
        else if(sscanf(argv[1],"%%%d",&jid) ==1 )
        {
            if(!(job = getjobjid(jobs,jid)))
            {
                printf("%d:No such job\n",jid);
                return ;
            } 
            else
            {
                if(strcmp("bg",argv[0])==0) 
                {
                    //pid<-1时，信号将送往以-pid为组标识的进程。
                    if(kill(-(job->pid), SIGCONT)<0) //kill为进程产生信号,而且信号不会被阻塞.
                        unix_error("do_bgfg: Kill error");
                    job->state = BG;
                }
                else if(strcmp("fg",argv[0])==0)
                {
                    if(kill(-(job->pid), SIGCONT)<0)
                        unix_error("do_bgfg: Kill error");
                    job->state = FG;
                    waitfg(job->pid);//等待fg完成
                }   
            }
        }
        else 
            printf("%s command requires right PID or jobid argument.\n", argv[0]);
    } 
    return;
}
/* 
 * waitfg - Block until process pid is no longer the foreground process
 * 等待fg进程结束.sigsuspend()???
 */
void waitfg(pid_t pid)
{
    while(fgpid(jobs)==pid)
        sleep(0);//CPU竞争,让其它进程执行, 
    //waitpid(pid,NULL,0);    /*this is wrong answer ,see the num5 hints*/
    return;
}

/*****************
 * Signal handlers
 *****************/

/* 
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.  
 */
void sigchld_handler(int sig) 
{
    //call waitpid (WUNTRACED 和 WNOHANG)
    pid_t pid;
    struct job_t * job;
    int status;
    sigset_t mask_all,prev_all;
    sigfillset(&mask_all);

    while((pid = waitpid(-1, &status, WNOHANG | WUNTRACED))>0)
    {
        if(WIFSTOPPED(status))/*process is stop because of a signal*/
        {
             if(!(job = getjobpid(jobs,pid)))
            {
                //jid = pid2jid(pid);
                printf("%d:No such job\n",pid);
                return ;
            }
            job->state = ST;
            printf("[%d] Stopped %s\n", (job->jid), (job->cmdline));
        }
        else if(WIFEXITED(status))/*process is exited in normal way*/
        {
            sigprocmask(SIG_BLOCK,&mask_all,&prev_all);
            deletejob(jobs,pid);
            sigprocmask(SIG_SETMASK,&prev_all,NULL);
        }
        else if(WIFSIGNALED(status))/*process is terminated by a signal*/
        {
            sigprocmask(SIG_BLOCK,&mask_all,&prev_all);
            deletejob(jobs,pid);
            sigprocmask(SIG_SETMASK,&prev_all,NULL);
            printf("Job  (%d) terminated by signal %d\n", pid, WTERMSIG(status));
        }
        else
            /* unix error, print message */
            unix_error("waitpid error");
        
    }
    return;
}

/* 
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.  
 */
void sigint_handler(int sig) 
{   
    pid_t pid ;
    if((pid=fgpid(jobs))>0)
    {
        if(kill(-pid,SIGINT)<0)
            unix_error("SIGINT: Kill error");
    }
    return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.  
 */
void sigtstp_handler(int sig) 
{
    pid_t pid ;
    if((pid=fgpid(jobs))>0)
    {
        struct job_t * job = getjobpid(jobs,pid);
        if(job->state == ST)
            return ;
        if(kill(-pid,SIGTSTP)<0)
            unix_error("SIGTSTP: Kill error");
    }
    return;
}

/*********************
 * End signal handlers
 *********************/

/***********************************************
 * Helper routines that manipulate the job list
 **********************************************/

/* clearjob - Clear the entries in a job struct */
void clearjob(struct job_t *job) {
    job->pid = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list */
void initjobs(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	clearjob(&jobs[i]);
}

/* maxjid - Returns largest allocated job ID */
int maxjid(struct job_t *jobs) 
{
    int i, max=0;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid > max)
	    max = jobs[i].jid;
    return max;
}

/* addjob - Add a job to the job list */
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline) 
{
    int i;
    if (pid < 1)
	    return 0;
    for (i = 0; i < MAXJOBS; i++) 
    {
        if (jobs[i].pid == 0) 
        {
            jobs[i].pid = pid;
            jobs[i].state = state;
            jobs[i].jid = nextjid++;
            if (nextjid > MAXJOBS)
                nextjid = 1;
            strcpy(jobs[i].cmdline, cmdline);
            if(verbose){
                printf("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
                }
                return 1;
        }
    }
    printf("Tried to create too many jobs\n");
    return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
int deletejob(struct job_t *jobs, pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid == pid) {
	    clearjob(&jobs[i]);
	    nextjid = maxjid(jobs)+1;
	    return 1;
	}
    }
    return 0;
}

/* fgpid - Return PID of current foreground job, 0 if no such job */
pid_t fgpid(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].state == FG)
	    return jobs[i].pid;
    return 0;
}

/* getjobpid  - Find a job (by PID) on the job list */
struct job_t *getjobpid(struct job_t *jobs, pid_t pid) {
    int i;

    if (pid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid)
	    return &jobs[i];
    return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
struct job_t *getjobjid(struct job_t *jobs, int jid) 
{
    int i;

    if (jid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid == jid)
	    return &jobs[i];
    return NULL;
}

/* pid2jid - Map process ID to job ID */
int pid2jid(pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid) {
            return jobs[i].jid;
        }
    return 0;
}

/* listjobs - Print the job list */
void listjobs(struct job_t *jobs) 
{
    int i;
    
    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid != 0) {
	    printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
	    switch (jobs[i].state) {
		case BG: 
		    printf("Running ");
		    break;
		case FG: 
		    printf("Foreground ");
		    break;
		case ST: 
		    printf("Stopped ");
		    break;
	    default:
		    printf("listjobs: Internal error: job[%d].state=%d ", 
			   i, jobs[i].state);
	    }
	    printf("%s", jobs[i].cmdline);
	}
    }
}
/******************************
 * end job list helper routines
 ******************************/


/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void) 
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * unix_error - unix-style error routine
 */
void unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * app_error - application-style error routine
 */
void app_error(char *msg)
{
    fprintf(stdout, "%s\n", msg);
    exit(1);
}

/*
 * Signal - wrapper for the sigaction function
 */
handler_t *Signal(int signum, handler_t *handler) 
{
    struct sigaction action, old_action;

    action.sa_handler = handler;  
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
	    unix_error("Signal error");
    return (old_action.sa_handler);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig) 
{
    printf("Terminating after receipt of SIGQUIT signal\n");
    exit(1);
}




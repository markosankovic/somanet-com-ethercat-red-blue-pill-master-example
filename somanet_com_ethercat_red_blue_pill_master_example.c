#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

#include <ecrt.h>

// Application parameters
#define FREQUENCY 100
#define PRIORITY 1

// Optional features
#define CONFIGURE_PDOS 0

// Pills
#define RED_PILL 0x0001
#define BLUE_PILL 0x0002

unsigned char pill = RED_PILL; 

// EtherCAT
static ec_master_t *master = NULL;
static ec_domain_t *domain1 = NULL;

// Timer
static unsigned int sig_alarms = 0;
static unsigned int user_alarms = 0;

// process data
static uint8_t *domain1_pd = NULL;

#define SomanetPos 0, 0

#define Somanet_EtherCAT 0x000022d2, 0x00000201

// offsets for PDO entries
static unsigned int off_pdo1_in;
static unsigned int off_pdo1_out;

const static ec_pdo_entry_reg_t domain1_regs[] = {
    {SomanetPos, Somanet_EtherCAT, 0x2000, 0x00, &off_pdo1_in, NULL},
    {SomanetPos, Somanet_EtherCAT, 0x2001, 0x00, &off_pdo1_out, NULL},
    {0}
};

#if CONFIGURE_PDOS
ec_pdo_entry_info_t slave_0_pdo_entries[] = {
    {0x2001, 0x00, 16}, /* Pill taken */
    {0x2000, 0x00, 16}, /* Offered pill */
};

ec_pdo_info_t slave_0_pdos[] = {
    {0x1a00, 1, slave_0_pdo_entries + 0}, /* Inputs */
    {0x1600, 1, slave_0_pdo_entries + 1}, /* Outputs */
};

ec_sync_info_t slave_0_syncs[] = {
    {0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
    {1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE},
    {2, EC_DIR_OUTPUT, 1, slave_0_pdos + 0, EC_WD_DISABLE},
    {3, EC_DIR_INPUT, 1, slave_0_pdos + 1, EC_WD_DISABLE},
    {0xff}
};
#endif

void cyclic_task() {
    // receive process data
    ecrt_master_receive(master);
    ecrt_domain_process(domain1);

    EC_WRITE_U16(domain1_pd + off_pdo1_out, pill);

    // send process data
    ecrt_domain_queue(domain1);
    ecrt_master_send(master);
}

void signal_handler(int signum) {
    switch (signum) {
        case SIGALRM:
            sig_alarms++;
            break;
        case SIGUSR1:
            pill = RED_PILL;
            printf("Master is now sending red pill.\n");
            break; 
        case SIGUSR2:
            pill = BLUE_PILL;
            printf("Master is now sending blue pill.\n");
            break; 
    }
}

int main(void) {
    pid_t pid = getpid();
    printf("Pid: %d\n", pid);

    ec_slave_config_t *sc;
    struct sigaction sa;
    struct itimerval tv;

    master = ecrt_request_master(0);
    if (!master)
        return -1;

    domain1 = ecrt_master_create_domain(master);
    if (!domain1)
        return -1;

    sc = ecrt_master_slave_config(master, SomanetPos, Somanet_EtherCAT);
    if (!sc)
        return -1;

#if CONFIGURE_PDOS
    if (ecrt_slave_config_pdos(sc, EC_END, slave_0_syncs)) {
        fprintf(stderr, "Failed to configure PDOs.\n");
        return -1;
    }
#endif

    if (ecrt_domain_reg_pdo_entry_list(domain1, domain1_regs)) {
        fprintf(stderr, "PDO entry registration failed!\n");
        return -1;
    }

    printf("Activating master...\n");
    if (ecrt_master_activate(master))
        return -1;

    if (!(domain1_pd = ecrt_domain_data(domain1))) {
        fprintf(stderr, "Failed to return the domain's process data!\n");
        return -1;
    }

#if PRIORITY
    if (setpriority(PRIO_PROCESS, pid, -19)) {
        fprintf(stderr, "Warning: Failed to set priority: %s\n", strerror(errno));
    }
#endif

    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGALRM, &sa, 0)) {
        fprintf(stderr, "Failed to install SIGALRM signal handler!\n");
        return -1;
    }

    if (sigaction(SIGUSR1, &sa, 0)) {
        fprintf(stderr, "Failed to install SIGUSR1 signal handler!\n");
        return -1;
    }

    if (sigaction(SIGUSR2, &sa, 0)) {
        fprintf(stderr, "Failed to install SIGUSR2 signal handler!\n");
        return -1;
    }

    printf("Starting timer...\n");
    tv.it_interval.tv_sec = 0;
    tv.it_interval.tv_usec = 1000000 / FREQUENCY;
    tv.it_value.tv_sec = 0;
    tv.it_value.tv_usec = 1000;
    if (setitimer(ITIMER_REAL, &tv, NULL)) {
        fprintf(stderr, "Failed to start timer: %s\n", strerror(errno));
        return 1;
    }

    printf("Started.\n");
    printf("Master is now sending red pill.\n");
    printf("Control master by sending signal to this process:\n");
    printf("\tkill -SIGUSR1 %d # send red pill\n", pid);
    printf("\tkill -SIGUSR2 %d # send blue pill\n", pid);

    while (1) {
        pause();

        while (sig_alarms != user_alarms) {
            cyclic_task();
            user_alarms++;
        }
    }

    return 0;
}


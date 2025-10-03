#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sched.h>

#define BLACK   "\033[30m"      // Black
#define RED     "\033[31m"      // Red
#define GREEN   "\033[32m"      // Green
#define YELLOW  "\033[33m"      // Yellow
#define BLUE    "\033[34m"      // Blue
#define MAGENTA "\033[35m"      // Magenta (Purple)
#define CYAN    "\033[36m"      // Cyan (Light Blue)
#define WHITE   "\033[37m"      // White
#define RESET   "\033[0m"       // Reset to default

typedef struct {
    int tid;
    int policy;
    int priority;
    char *color;
} TaskParams;

void print_usage(const char *program_name) {
    printf("Usage: %s --tid <id> --policy <policy> --priority <value> --color <color>\n", program_name);
    printf("\nOptions:\n");
    printf("  -t, --tid <id>        Task ID (required)\n");
    printf("  -p, --policy <policy> Scheduling policy (other, fifo, rr)\n");
    printf("  -r, --priority <val>  Priority value\n");
    printf("  -c, --color <color>   Display color (black, red, green, yellow, blue, magenta, cyan, white)\n");
    printf("  -h, --help            Display this help message\n");

  }

/* Parse function for policy */
int parse_policy(const char *policy_str) {
    if (strcmp(policy_str, "other") == 0 || strcmp(policy_str, "OTHER") == 0) {
        return SCHED_OTHER;
    } else if (strcmp(policy_str, "fifo") == 0 || strcmp(policy_str, "FIFO") == 0) {
        return SCHED_FIFO;
    } else if (strcmp(policy_str, "rr") == 0 || strcmp(policy_str, "RR") == 0) {
        return SCHED_RR;
    }
    return -1; // Invalid policy
}

const char *policy_to_string(int policy) {
    switch (policy) {
        case SCHED_OTHER: return "SCHED_OTHER";
        case SCHED_FIFO:  return "SCHED_FIFO";
        case SCHED_RR:    return "SCHED_RR";
        default:          return "UNKNOWN";
    }
}

/* Parse function for colors */
const char *parse_color(const char *color_str) {
    if (strcasecmp(color_str, "black") == 0) {
        return "\033[30m";
    } else if (strcasecmp(color_str, "red") == 0) {
        return "\033[31m";
    } else if (strcasecmp(color_str, "green") == 0) {
        return "\033[32m";
    } else if (strcasecmp(color_str, "yellow") == 0) {
        return "\033[33m";
    } else if (strcasecmp(color_str, "blue") == 0) {
        return "\033[34m";
    } else if (strcasecmp(color_str, "magenta") == 0 || strcasecmp(color_str, "purple") == 0) {
        return "\033[35m";
    } else if (strcasecmp(color_str, "cyan") == 0 || strcasecmp(color_str, "light blue") == 0) {
        return "\033[36m";
    } else if (strcasecmp(color_str, "white") == 0) {
        return "\033[37m";
    }
    return NULL; // Invalid color
}

int parse_arguments(int argc, char *argv[], TaskParams *params) {
    // Initialize with default values
    params->tid = -1;
    params->policy = SCHED_OTHER; // Default policy
    params->priority = 0;
    params->color = RESET;

    // Define long options
    static struct option long_options[] = {
        {"tid", required_argument, 0, 't'},
        {"policy", required_argument, 0, 'p'},
        {"priority", required_argument, 0, 'r'},
        {"color", required_argument, 0, 'c'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "t:p:r:c:h", long_options, &option_index)) != -1) {
        
        switch (opt) {
            case 't':
                params->tid = atoi(optarg);
                break;
            case 'p': 
                printf("%s", optarg);
                int policy = parse_policy(optarg);
                if (policy == -1) {
                    fprintf(stderr, "Error: Invalid policy '%s'. Valid options: other, fifo, rr\n", optarg);
                    return 0;
                }
                params->policy = policy;
                break;
            
            case 'r':
                params->priority = atoi(optarg);
                break;
            case 'c':
                params->color = parse_color(optarg);
                if (params->color == NULL) {
                  fprintf(stderr, "Error: Invalid colors '%s'. Valid options: black, red, green, yellow, blue, magenta, cyan, white\n", optarg);
                  return 0; 
                }
                break;
            case 'h':
                print_usage(argv[0]);
                exit(EXIT_SUCCESS);
            case '?':
                // getopt_long already printed an error message
                return 0;
            default:
                fprintf(stderr, "Error: Unknown option\n");
                return 0;
        }
    }

    // Validate required parameters
    if (params->tid == -1) {
        fprintf(stderr, "Error: --tid is required\n");
        return 0;
    }

    return 1;
}


int main(int argc, char **argv){
  
  /* Manage input parameters */
  TaskParams params;
  
    if (argc < 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (!parse_arguments(argc, argv, &params)) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }



  /* Setting scheduler policy and priority*/
  struct sched_param param;
  int thread_id = params.tid;
  param.sched_priority = params.priority; 
  sched_setscheduler(0, params.policy, &param);
 
  
  /* Infinity loop */
  long long int n = 0;
  while(1) {
    n = n + 1;
    if (!(n % 10000000)) {

      /* Get the CPU Core used*/
      int current_cpu = sched_getcpu();

      /* Print: CPU, Thread ID, Sched Policy, Priority, Timer(n)*/
      printf("%s%d,%d,%s,%d,%llu%s\n", params.color, current_cpu, thread_id, policy_to_string(params.policy), params.priority, n, RESET);

    }
  }
}
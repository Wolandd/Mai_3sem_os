#include "common.h"

static volatile sig_atomic_t start_flag = 0;

void handle_start(int sig) {
    (void)sig;
    start_flag = 1;
}

void* open_shared_memory(const char* name, size_t size) {
    int shm_fd;
    void* addr;
    
    shm_fd = shm_open(name, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return NULL;
    }
    
    addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        return NULL;
    }
    
    close(shm_fd);
    return addr;
}

void close_shared_memory(void* addr, size_t size) {
    if (addr != NULL && addr != MAP_FAILED) {
        if (munmap(addr, size) == -1) {
            perror("munmap");
        }
    }
}

int main(int argc, char* argv[]) {
    char* filename;
    void* shm_addr = NULL;
    char* numbers_str;
    float numbers[100];
    int count = 0;
    float result;
    FILE* output_file;
    pid_t parent_pid;
    
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <result_file_name>\n", argv[0]);
        return 1;
    }
    
    filename = argv[1];
    parent_pid = getppid();
    
    shm_addr = open_shared_memory(SHM_NAME, SHM_SIZE);
    if (shm_addr == NULL) {
        fprintf(stderr, "Failed to open shared memory\n");
        return 1;
    }
    
    signal(SIGNAL_START, handle_start);
    
    if (kill(parent_pid, SIGNAL_READY) == -1) {
        perror("kill");
        close_shared_memory(shm_addr, SHM_SIZE);
        return 1;
    }
    
    while (!start_flag) {
        pause();
    }
    
    numbers_str = (char*)shm_addr;
    
    {
        char* ptr = numbers_str;
        char* endptr;
        float num;
        
        while (*ptr != '\0' && count < 100) {
            while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n') {
                ptr++;
            }
            
            if (*ptr == '\0') {
                break;
            }
            
            num = strtof(ptr, &endptr);
            
            if (endptr == ptr) {
                fprintf(stderr, "Error: invalid number format\n");
                close_shared_memory(shm_addr, SHM_SIZE);
                return 1;
            }
            
            numbers[count] = num;
            count++;
            ptr = endptr;
        }
    }
    
    if (count < 2) {
        fprintf(stderr, "Error: at least 2 numbers required\n");
        close_shared_memory(shm_addr, SHM_SIZE);
        return 1;
    }
    
    result = numbers[0];
    for (int i = 1; i < count; i++) {
        if (numbers[i] == 0.0f) {
            fprintf(stderr, "Error: division by zero\n");
            close_shared_memory(shm_addr, SHM_SIZE);
            return 1;
        }
        result /= numbers[i];
    }
    
    output_file = fopen(filename, "w");
    if (output_file == NULL) {
        perror("fopen");
        close_shared_memory(shm_addr, SHM_SIZE);
        return 1;
    }
    
    if (fprintf(output_file, "%.6f\n", result) < 0) {
        perror("fprintf");
        fclose(output_file);
        close_shared_memory(shm_addr, SHM_SIZE);
        return 1;
    }
    
    if (fclose(output_file) != 0) {
        perror("fclose");
        close_shared_memory(shm_addr, SHM_SIZE);
        return 1;
    }
    
    if (kill(parent_pid, SIGNAL_READY) == -1) {
        perror("kill");
        close_shared_memory(shm_addr, SHM_SIZE);
        return 1;
    }
    
    close_shared_memory(shm_addr, SHM_SIZE);
    
    return 0;
}

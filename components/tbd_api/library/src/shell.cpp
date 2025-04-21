#include <tbd/api/shell.hpp>

#include <cstdio>
#include <sys/unistd.h>
#include <iostream>

extern "C" {
#include <microshell.h>
}

#include <tbd/storage/file_system.hpp>
#include <tbd/system/tasks.hpp>


// working buffers allocations (size could be customized)
#define BUF_IN_SIZE    32
#define BUF_OUT_SIZE   32
#define PATH_MAX_SIZE  32

const char* root_path = "/spiffs";
std::string current_workdir(root_path);


void cd_cmd(ush_object *self, ush_file_descriptor const *file, int argc, char *argv[]) {
    namespace fs = tbd::storage::filesystem;
    if (self->current_node == nullptr || self->current_node->path == nullptr) {
        return;
    }

    fs::path workdir(self->current_node->path);
    for (const auto& path : fs::directory_iterator(workdir)) {
        std::cout << path.path().filename() << std::endl;
    }
}


void ls_cmd(ush_object *self, ush_file_descriptor const *file, int argc, char *argv[]) {
    namespace fs = tbd::storage::filesystem;
    if (self->current_node == nullptr || self->current_node->path == nullptr) {
        return;
    }

    fs::path workdir(self->current_node->path);
    for (const auto& path : fs::directory_iterator(workdir)) {
        std::cout << path.path().filename() << std::endl;
    }
}


// non-blocking read interface
static int shell_read_char(struct ush_object *self, char *ch) {
    *ch = getchar();
    if(*ch == '\0' || *ch == 0xFF || *ch == '\r') {
        return 0;
    }
    return 1;
}

// non-blocking write interface
static int shell_write_char(struct ush_object *self, char ch) {
    return putchar(ch) != EOF;
}

// I/O interface descriptor
const ush_io_interface ush_iface = {
    .read = shell_read_char,
    .write = shell_write_char,
};

char ush_in_buf[BUF_IN_SIZE];
char ush_out_buf[BUF_OUT_SIZE];

// microshell instance handler
ush_object ush;

// microshell descriptor
const ush_descriptor ush_desc = {
    .io = &ush_iface,                           // I/O interface pointer
    .input_buffer = ush_in_buf,                 // working input buffer
    .input_buffer_size = sizeof(ush_in_buf),    // working input buffer size
    .output_buffer = ush_out_buf,               // working output buffer
    .output_buffer_size = sizeof(ush_out_buf),  // working output buffer size
    .path_max_length = PATH_MAX_SIZE,           // path maximum length (stack)
    .hostname = "tbd",                      // hostname (in prompt)
};

ush_file_descriptor commands[] = {
    {
        .name = "foo",
        .description = "list files",
        .help = nullptr,
        .exec = ls_cmd,
    },
    {
        .name = "foo",
        .description = "list files",
        .help = nullptr,
        .exec = ls_cmd,
    },
};

ush_node_object cmd;

// root directory handler
ush_node_object root = {
    .file_list = nullptr,
    .file_list_size =  0,
    .path = root_path,
    .parent = nullptr,
    .children = nullptr,
    .next = nullptr,
};

namespace tbd::api {

void Shell::begin() {
    fflush(stdout);
    fsync(fileno(stdout));

    ush_init(&ush, &ush_desc);
    ush.root = &root;
    ush.current_node = &root;
    ush_commands_add(&ush, &cmd, commands, sizeof(commands) / sizeof(commands[0]));

    // if (ush_node_mount(&ush, "/spiffs", &root, NULL, 0) != USH_STATUS_OK) {
    //     ESP_LOGE("shell", "failed to set shell base path");
    //     return;
    // }

    while (true) {
        ush_service(&ush);
        // char ch;
        // if (shell_read_char(nullptr, &ch)) {
        //     shell_write_char(nullptr, ch);
        // }

        system::Task::sleep(50);
    }
}

void Shell::end() {

}

}
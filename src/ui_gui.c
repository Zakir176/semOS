#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui_gui.h"
#include "process.h"
#include "scheduler.h"
#include "memory.h"
#include "ipc.h"
#include "deadlock.h"
#include "filemanager.h"
#include "logger.h"

static GtkWidget *mainWindow;
static GtkWidget *logView;

static void appendLog(const char *message) {
    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(logView));
    GtkTextIter    end;
    gtk_text_buffer_get_end_iter(buf, &end);
    gtk_text_buffer_insert(buf, &end, message, -1);
    gtk_text_buffer_insert(buf, &end, "\n", -1);

    GtkTextMark *mark = gtk_text_buffer_get_mark(buf, "insert");
    gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(logView), mark);
}

static void showInfo(const char *title, const char *msg) {
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(mainWindow),
        GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg);
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void showError(const char *msg) {
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(mainWindow),
        GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", msg);
    gtk_window_set_title(GTK_WINDOW(dialog), "Error");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static int promptInt(const char *label) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(label,
        GTK_WINDOW(mainWindow), GTK_DIALOG_MODAL,
        "OK", GTK_RESPONSE_OK, NULL);

    GtkWidget *entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                       gtk_label_new(label), FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                       entry, FALSE, FALSE, 4);
    gtk_widget_show_all(dialog);

    int val = 0;
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK)
        val = atoi(gtk_entry_get_text(GTK_ENTRY(entry)));

    gtk_widget_destroy(dialog);
    return val;
}

static char *promptString(const char *label) {
    static char buf[256];
    GtkWidget *dialog = gtk_dialog_new_with_buttons(label,
        GTK_WINDOW(mainWindow), GTK_DIALOG_MODAL,
        "OK", GTK_RESPONSE_OK, NULL);

    GtkWidget *entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                       gtk_label_new(label), FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                       entry, FALSE, FALSE, 4);
    gtk_widget_show_all(dialog);

    buf[0] = '\0';
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK)
        strncpy(buf, gtk_entry_get_text(GTK_ENTRY(entry)), sizeof(buf) - 1);

    gtk_widget_destroy(dialog);
    return buf;
}

/* ────────────── Process Tab ────────────── */

static void onCreateProcess(GtkWidget *btn, gpointer data) {
    (void)btn; (void)data;
    char *name    = promptString("Process Name:");
    int   burst   = promptInt("Burst Time:");
    int   arrival = promptInt("Arrival Time:");
    int   prio    = promptInt("Priority (1=Low 2=Medium 3=High):");
    int   mem     = promptInt("Memory Required (KB):");

    if (prio < 1 || prio > 3) { showError("Invalid priority (1-3)."); return; }
    if (burst <= 0 || mem <= 0) { showError("Burst time and memory must be > 0."); return; }

    int pid = createProcess(name, burst, arrival, (ProcessPriority)prio, mem);
    if (pid > 0) {
        char msg[128];
        snprintf(msg, sizeof(msg), "[PROCESS] Created '%s' PID=%d", name, pid);
        appendLog(msg);
    } else {
        showError("Failed to create process.");
    }
}

static void onTerminateProcess(GtkWidget *btn, gpointer data) {
    (void)btn; (void)data;
    int pid = promptInt("PID to terminate:");
    if (terminateProcess(pid) == 0) {
        char msg[64];
        snprintf(msg, sizeof(msg), "[PROCESS] Terminated PID=%d", pid);
        appendLog(msg);
    } else {
        showError("Termination failed: PID not found.");
    }
}

static void onListProcesses(GtkWidget *btn, gpointer data) {
    (void)btn; (void)data;
    int   count;
    PCB **procs = getAllProcesses(&count);

    if (count == 0) { showInfo("Processes", "No active processes."); return; }

    GString *out = g_string_new("PID    Name                 State        Priority\n");
    g_string_append(out, "------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        g_string_append_printf(out, "%-6d %-20s %-12s %s\n",
            procs[i]->pid, procs[i]->name,
            stateToString(procs[i]->state),
            priorityToString(procs[i]->priority));
    }
    showInfo("Active Processes", out->str);
    g_string_free(out, TRUE);
}

static GtkWidget *buildProcessTab(void) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(box), 12);

    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<b>Process Management</b>");
    gtk_box_pack_start(GTK_BOX(box), title, FALSE, FALSE, 0);

    GtkWidget *btnCreate = gtk_button_new_with_label("Create Process");
    GtkWidget *btnTerm   = gtk_button_new_with_label("Terminate Process");
    GtkWidget *btnList   = gtk_button_new_with_label("List Processes");

    g_signal_connect(btnCreate, "clicked", G_CALLBACK(onCreateProcess), NULL);
    g_signal_connect(btnTerm,   "clicked", G_CALLBACK(onTerminateProcess), NULL);
    g_signal_connect(btnList,   "clicked", G_CALLBACK(onListProcesses), NULL);

    gtk_box_pack_start(GTK_BOX(box), btnCreate, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), btnTerm,   FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), btnList,   FALSE, FALSE, 0);
    return box;
}

/* ────────────── Scheduler Tab ────────────── */

static void runScheduler(int algo) {
    int   count;
    PCB **procs = getAllProcesses(&count);
    if (count == 0) { showInfo("Scheduler", "No active processes to schedule."); return; }

    ScheduleResult result;
    const char    *algoName;

    if (algo == 0) {
        result   = runFCFS(procs, count);
        algoName = "FCFS";
    } else if (algo == 1) {
        result   = runSJF(procs, count);
        algoName = "SJF";
    } else {
        int q = promptInt("Time Quantum:");
        result   = runRoundRobin(procs, count, q);
        algoName = "Round Robin";
    }

    GString *out = g_string_new(NULL);
    g_string_append_printf(out, "Algorithm: %s\n\n", algoName);
    g_string_append(out, "PID    Name                 Waiting  Turnaround\n");
    g_string_append(out, "------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        g_string_append_printf(out, "%-6d %-20s %-8d %d\n",
            procs[i]->pid, procs[i]->name,
            procs[i]->waitingTime, procs[i]->turnaroundTime);
    }
    g_string_append_printf(out, "\nAvg Waiting Time    : %.2f\n", result.avgWaitingTime);
    g_string_append_printf(out, "Avg Turnaround Time : %.2f\n", result.avgTurnaroundTime);
    g_string_append_printf(out, "CPU Utilization     : %.2f%%\n", result.cpuUtilization);

    char logMsg[64];
    snprintf(logMsg, sizeof(logMsg), "[SCHEDULER] %s completed", algoName);
    appendLog(logMsg);
    showInfo("Scheduling Results", out->str);
    g_string_free(out, TRUE);
}

static GtkWidget *buildSchedulerTab(void) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(box), 12);

    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<b>CPU Scheduling</b>");
    gtk_box_pack_start(GTK_BOX(box), title, FALSE, FALSE, 0);

    GtkWidget *btnFCFS = gtk_button_new_with_label("Run FCFS");
    GtkWidget *btnSJF  = gtk_button_new_with_label("Run SJF");
    GtkWidget *btnRR   = gtk_button_new_with_label("Run Round Robin");

    g_signal_connect_swapped(btnFCFS, "clicked",
        G_CALLBACK(runScheduler), GINT_TO_POINTER(0));
    g_signal_connect_swapped(btnSJF,  "clicked",
        G_CALLBACK(runScheduler), GINT_TO_POINTER(1));
    g_signal_connect_swapped(btnRR,   "clicked",
        G_CALLBACK(runScheduler), GINT_TO_POINTER(2));

    gtk_box_pack_start(GTK_BOX(box), btnFCFS, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), btnSJF,  FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), btnRR,   FALSE, FALSE, 0);
    return box;
}

/* ────────────── Memory Tab ────────────── */

static void onAllocateMemory(GtkWidget *btn, gpointer data) {
    (void)btn; (void)data;
    int pid      = promptInt("PID:");
    int size     = promptInt("Size (KB):");
    int strategy = promptInt("Strategy (0=First 1=Best 2=Worst):");

    if (strategy < 0 || strategy > 2) { showError("Invalid strategy."); return; }
    int addr = allocateMemory(pid, size, (FitStrategy)strategy);
    if (addr >= 0) {
        char msg[128];
        snprintf(msg, sizeof(msg), "[MEMORY] Allocated %d KB to PID=%d at 0x%04X", size, pid, addr);
        appendLog(msg);
    } else {
        showError("Allocation failed: insufficient memory.");
    }
}

static void onFreeMemory(GtkWidget *btn, gpointer data) {
    (void)btn; (void)data;
    int pid = promptInt("PID to free:");
    if (freeMemory(pid) == 0) {
        char msg[64];
        snprintf(msg, sizeof(msg), "[MEMORY] Freed memory for PID=%d", pid);
        appendLog(msg);
    } else {
        showError("Free failed: PID not in memory.");
    }
}

static void onViewMemoryStats(GtkWidget *btn, gpointer data) {
    (void)btn; (void)data;
    MemoryStats s = getMemoryStats();
    GString *out  = g_string_new(NULL);
    g_string_append_printf(out, "Total Memory   : %d KB\n", s.totalMemory);
    g_string_append_printf(out, "Used Memory    : %d KB\n", s.usedMemory);
    g_string_append_printf(out, "Free Memory    : %d KB\n", s.freeMemory);
    g_string_append_printf(out, "Partitions     : %d\n",    s.partitionCount);
    g_string_append_printf(out, "Free Fragments : %d\n",    s.fragmentCount);
    g_string_append_printf(out, "Utilization    : %.1f%%\n",
        s.totalMemory > 0 ? (float)s.usedMemory / s.totalMemory * 100.0f : 0.0f);
    showInfo("Memory Statistics", out->str);
    g_string_free(out, TRUE);
}

static GtkWidget *buildMemoryTab(void) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(box), 12);

    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<b>Memory Management</b>");
    gtk_box_pack_start(GTK_BOX(box), title, FALSE, FALSE, 0);

    GtkWidget *btnAlloc = gtk_button_new_with_label("Allocate Memory");
    GtkWidget *btnFree  = gtk_button_new_with_label("Free Memory");
    GtkWidget *btnStats = gtk_button_new_with_label("View Memory Statistics");

    g_signal_connect(btnAlloc, "clicked", G_CALLBACK(onAllocateMemory), NULL);
    g_signal_connect(btnFree,  "clicked", G_CALLBACK(onFreeMemory),     NULL);
    g_signal_connect(btnStats, "clicked", G_CALLBACK(onViewMemoryStats), NULL);

    gtk_box_pack_start(GTK_BOX(box), btnAlloc, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), btnFree,  FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), btnStats, FALSE, FALSE, 0);
    return box;
}

/* ────────────── IPC Tab ────────────── */

static void onRunIPC(GtkWidget *btn, gpointer data) {
    (void)btn;
    int which = GPOINTER_TO_INT(data);

    if (which == 0)      runPipeDemo();
    else if (which == 1) runMessageQueueDemo();
    else                 runSharedMemoryDemo();

    appendLog("[IPC] Demo completed — see terminal for output.");
}

static GtkWidget *buildIPCTab(void) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(box), 12);

    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<b>IPC Mechanisms</b>");
    gtk_box_pack_start(GTK_BOX(box), title, FALSE, FALSE, 0);

    GtkWidget *note = gtk_label_new("IPC demos run in the terminal window.");
    gtk_box_pack_start(GTK_BOX(box), note, FALSE, FALSE, 4);

    GtkWidget *btnPipe = gtk_button_new_with_label("Run Pipe Demo");
    GtkWidget *btnMQ   = gtk_button_new_with_label("Run Message Queue Demo");
    GtkWidget *btnSHM  = gtk_button_new_with_label("Run Shared Memory Demo");

    g_signal_connect(btnPipe, "clicked", G_CALLBACK(onRunIPC), GINT_TO_POINTER(0));
    g_signal_connect(btnMQ,   "clicked", G_CALLBACK(onRunIPC), GINT_TO_POINTER(1));
    g_signal_connect(btnSHM,  "clicked", G_CALLBACK(onRunIPC), GINT_TO_POINTER(2));

    gtk_box_pack_start(GTK_BOX(box), btnPipe, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), btnMQ,   FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), btnSHM,  FALSE, FALSE, 0);
    return box;
}

/* ────────────── Deadlock Tab ────────────── */

static BankerState guiState;

static void onRunSafety(GtkWidget *btn, gpointer data) {
    (void)btn; (void)data;
    int seq[MAX_DL_PROCESSES];
    int safe = runSafetyAlgorithm(&guiState, seq);

    GString *out = g_string_new(NULL);
    g_string_append_printf(out, "Status: %s\n\n", safe ? "SAFE" : "UNSAFE");
    if (safe) {
        g_string_append(out, "Safe Sequence: ");
        for (int i = 0; i < guiState.processCount; i++)
            g_string_append_printf(out, "P%d%s", seq[i],
                i < guiState.processCount - 1 ? " -> " : "");
    } else {
        g_string_append(out, "Deadlock risk detected.\nNo safe sequence exists.");
    }

    appendLog(safe ? "[DEADLOCK] Safety check: SAFE" : "[DEADLOCK] Safety check: UNSAFE");
    showInfo("Safety Algorithm", out->str);
    g_string_free(out, TRUE);
}

static void onDetectCycle(GtkWidget *btn, gpointer data) {
    (void)btn; (void)data;
    int cycle = detectCycle();
    appendLog(cycle ? "[DEADLOCK] Cycle detected — DEADLOCK" : "[DEADLOCK] No cycle detected");
    showInfo("Cycle Detection", cycle
        ? "DEADLOCK DETECTED\nCycle found in wait-for graph."
        : "No deadlock.\nNo cycle detected in wait-for graph.");
}

static void onAddWaitEdge(GtkWidget *btn, gpointer data) {
    (void)btn; (void)data;
    int from = promptInt("From process (P_n):");
    int to   = promptInt("To process (P_n):");
    addWaitEdge(from, to);
    char msg[64];
    snprintf(msg, sizeof(msg), "[DEADLOCK] Added edge P%d -> P%d", from, to);
    appendLog(msg);
}

static GtkWidget *buildDeadlockTab(void) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(box), 12);

    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<b>Deadlock Management</b>");
    gtk_box_pack_start(GTK_BOX(box), title, FALSE, FALSE, 0);

    GtkWidget *btnSafety = gtk_button_new_with_label("Run Safety Algorithm");
    GtkWidget *btnCycle  = gtk_button_new_with_label("Detect Cycle (RAG)");
    GtkWidget *btnEdge   = gtk_button_new_with_label("Add Wait-For Edge");

    g_signal_connect(btnSafety, "clicked", G_CALLBACK(onRunSafety),   NULL);
    g_signal_connect(btnCycle,  "clicked", G_CALLBACK(onDetectCycle), NULL);
    g_signal_connect(btnEdge,   "clicked", G_CALLBACK(onAddWaitEdge), NULL);

    gtk_box_pack_start(GTK_BOX(box), btnSafety, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), btnCycle,  FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), btnEdge,   FALSE, FALSE, 0);
    return box;
}

/* ────────────── File Manager Tab ────────────── */

static void onCreateFile(GtkWidget *btn, gpointer data) {
    (void)btn; (void)data;
    char *name = promptString("File Name:");
    int   type = promptInt("Type (0=Incident 1=Dispatch 2=Report 3=Log):");
    if (type < 0 || type > 3) { showError("Invalid file type."); return; }
    int id = createFile(name, (FileType)type, 0);
    if (id > 0) {
        char msg[128];
        snprintf(msg, sizeof(msg), "[FILEMGR] Created '%s' ID=%d", name, id);
        appendLog(msg);
    } else {
        showError("File creation failed.");
    }
}

static void onListFiles(GtkWidget *btn, gpointer data) {
    (void)btn; (void)data;
    GString *out = g_string_new("ID   Name                     Type       Size\n");
    g_string_append(out, "-----------------------------------------------\n");

    int found = 0;
    for (int i = 1; i < 64; i++) {
        VirtualFile *f = findFileById(i);
        if (!f) continue;
        g_string_append_printf(out, "%-4d %-24s %-10s %d\n",
            f->id, f->name, fileTypeToString(f->type), f->size);
        found = 1;
    }
    if (!found) g_string_append(out, "  No files in system.\n");
    showInfo("File System", out->str);
    g_string_free(out, TRUE);
}

static void onWriteFile(GtkWidget *btn, gpointer data) {
    (void)btn; (void)data;
    int   id      = promptInt("File ID:");
    char *content = promptString("Content:");

    if (openFile(id) != 0) { showError("Cannot open file."); return; }
    int n = writeFile(id, content);
    closeFile(id);

    if (n >= 0) {
        char msg[64];
        snprintf(msg, sizeof(msg), "[FILEMGR] Wrote %d bytes to ID=%d", n, id);
        appendLog(msg);
    } else {
        showError("Write failed.");
    }
}

static void onDeleteFile(GtkWidget *btn, gpointer data) {
    (void)btn; (void)data;
    int id = promptInt("File ID to delete:");
    if (deleteFile(id) == 0) {
        char msg[64];
        snprintf(msg, sizeof(msg), "[FILEMGR] Deleted file ID=%d", id);
        appendLog(msg);
    } else {
        showError("Delete failed: file not found.");
    }
}

static GtkWidget *buildFileTab(void) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(box), 12);

    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<b>File Management</b>");
    gtk_box_pack_start(GTK_BOX(box), title, FALSE, FALSE, 0);

    GtkWidget *btnCreate = gtk_button_new_with_label("Create File");
    GtkWidget *btnList   = gtk_button_new_with_label("List Files");
    GtkWidget *btnWrite  = gtk_button_new_with_label("Write to File");
    GtkWidget *btnDelete = gtk_button_new_with_label("Delete File");

    g_signal_connect(btnCreate, "clicked", G_CALLBACK(onCreateFile), NULL);
    g_signal_connect(btnList,   "clicked", G_CALLBACK(onListFiles),  NULL);
    g_signal_connect(btnWrite,  "clicked", G_CALLBACK(onWriteFile),  NULL);
    g_signal_connect(btnDelete, "clicked", G_CALLBACK(onDeleteFile), NULL);

    gtk_box_pack_start(GTK_BOX(box), btnCreate, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), btnList,   FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), btnWrite,  FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), btnDelete, FALSE, FALSE, 0);
    return box;
}

/* ────────────── Main Window ────────────── */

static GtkWidget *buildLogPanel(void) {
    GtkWidget *frame = gtk_frame_new("Event Log");
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scroll, -1, 120);

    logView = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(logView), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(logView), GTK_WRAP_WORD_CHAR);
    gtk_container_add(GTK_CONTAINER(scroll), logView);
    gtk_container_add(GTK_CONTAINER(frame), scroll);
    return frame;
}

void gui_run(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    initProcessManager();
    initMemory(MAX_MEMORY_KB);
    initFileManager();
    initDeadlock();
    memset(&guiState, 0, sizeof(guiState));

    mainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(mainWindow),
                         "SERC Mini-OS — Smart Emergency Response Center");
    gtk_window_set_default_size(GTK_WINDOW(mainWindow), 700, 560);
    gtk_container_set_border_width(GTK_CONTAINER(mainWindow), 8);
    g_signal_connect(mainWindow, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_add(GTK_CONTAINER(mainWindow), vbox);

    GtkWidget *header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header),
        "<span size='large' weight='bold'>Smart Emergency Response Center — Mini OS</span>\n"
        "<span size='small'>Copperbelt University | CS 225</span>");
    gtk_box_pack_start(GTK_BOX(vbox), header, FALSE, FALSE, 4);

    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 0);

    GtkWidget *notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
        buildProcessTab(),   gtk_label_new("Processes"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
        buildSchedulerTab(), gtk_label_new("Scheduler"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
        buildMemoryTab(),    gtk_label_new("Memory"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
        buildIPCTab(),       gtk_label_new("IPC"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
        buildDeadlockTab(),  gtk_label_new("Deadlock"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
        buildFileTab(),      gtk_label_new("Files"));

    gtk_box_pack_start(GTK_BOX(vbox), buildLogPanel(), FALSE, FALSE, 0);

    appendLog("SERC OS started in GUI mode.");
    logEvent("SYSTEM", "GUI session started");

    gtk_widget_show_all(mainWindow);
    gtk_main();

    logEvent("SYSTEM", "GUI session ended");
}

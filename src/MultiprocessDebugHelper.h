#ifndef DISTRIBUTEDMONITOR_MULTIPROCESSDEBUGHELPER_H
#define DISTRIBUTEDMONITOR_MULTIPROCESSDEBUGHELPER_H

/**
 * @author Krzysztof Wencel
 *
 * Attaches gdbserver to itself on a given port to make debugging with gdb (client) easier in distributed
 * environment like MPI.
 */
class MultiprocessDebugHelper {
public:
    /**
     * Attaches gdbserver to this program's process.
     * @param port gdbserver will listen on
     * @return gdbserver PID
     */
    static int setup(int port);

    MultiprocessDebugHelper() = delete;
    ~MultiprocessDebugHelper() = delete;

private:
    static void cleanup(int i);

    static int gdbServerPid;
};

#endif //DISTRIBUTEDMONITOR_MULTIPROCESSDEBUGHELPER_H

About
======
This is an example of Distributed Monitor implementation. 
It uses *MPI* as communication protocol and *Ricart-Agrawala* as communication algorithm.
Examples of monitor implementations are under ``examples`` directory. 

``TestBuffer`` shows how distributed mutex works.

``ConsumerProducerQueue`` is an implementation example of *Producer-Consumer Problem* in distributed version 
(both distributed monitor and distributed condition variable used).

Building and running project
============
Current directory: *NPR_Monitor*.

### Building
```commandline
cmake
make
```
### Running application
Example of running application on 2 machines (cores).
```commandline
mpirun -np 2 ./bin/NPR_Monitor
```

Implementing monitor
==================
### Inherit from DistributedMonitor
```objectivec
class ConsumerProducerQueue : public DistributedMonitor {
    (...)
};
```

### Create constructor

The only required argument is pointer to `ConnectionInterface` e.g. `MPI_Connection`.
```objectivec
ConsumerProducerQueue(std::shared_ptr<ConnectionInterface> connection) : 
    DistributedMonitor(connection) { }
```

### Oerride virtual functions
```objectivec
void manageReceivedData(std::string receivedData) override { 
    (...) 
}

std::string returnDataToSend() override {
    (...)
}
```
``manageReceivedData`` manages data (in ``std::string`` form)  received from node in distributed system e.g. saves it to local buffer.

``returnDataToSend`` transforms ready to share data into ``std::string``.

### Call `destruct()` in destructor
```objectivec
~ConsumerProducerQueue() {
    destruct();
    (...)
}
```

### Creating condition variables

All new condition variables (distributed versions) must be elements of ``d_cvMap`` to work properly.
Every ``DistributedConditionVariable`` has unique name (key in ``d_cvMap``) by which it is identified.
Name is the first argument in declaration (*"id1"* in example below).

Second argument is mutex of type ``DistributedMutex`` on which condition variable will be operating.
```objectivec
d_cvMap["id1"] = std::make_shared<DistributedConditionVariable>("id1", d_mutex);
```

Creating objects
=============
```objectivec
// 1)
bool Logger::showSystemLogs = true;

int main(int argc, char *argv[]) {
    // 2)
    std::shared_ptr<ConnectionInterface> connection = std::make_shared<MPI_Connection>(argc, argv, 4);
    
    // 3)
    ConsumerProducerQueue buffer(connection);
    ConsumerProducerQueue buffer2(connection);
    
    // 4)
    (...)
    buffer.produce(3);
    buffer2.consume();
    (...)
    
    // 5)
    buffer.endCommunication();
    buffer2.endCommunication();
    
    // 6)
    MPI_Connection::endConnection();
    return 0;
}
```
1) Inform if you want to see logs about mutex locking and waiting on condition variable.
2) Create communicator object. In example above ``MPI_Connection`` is used.
The last argument is unique id for communicator. It is possible to create more communicators in one project.
2) Create objects of implemented Monitor. 
If you want them to communicate with each other, give them the same communicator in constructor.
3) Do what you want.
4) Call ``endCommunication()`` method on every created monitor object.
5) If you use ``MPI_Connection`` as communicator you have to call `` MPI_Connection::endConnection()`` at 
the very end of program (end of ``main`` function).

>Monitors communicate with each other using ``ConnectionInterface``. 
If you want to lock mutex or wait on condition variable, you have to remember that they work 
in **distributed** system. They **do not** work locally like common mutual exclusion mechanisms. 
In consequence, if you use the same object in multiple threads, the synchronization won't work properly! 
>

Logger
=====
### Unique monitor's id
Every declared monitor has unique *id* in distributed system.
Three elements make up one id e.g. ``100:0:4``. 

`4` identify unique communicator id (passed in ``ConnectionInterface`` constructor).

`0` identify distributed id in communicator. 
In ``MPI_Communication`` it is *MPI_Comm_rank*.

`100` identify monitor in local system. This number grows with step equal 100.

### Logging
##### System logs
To turn off/on system logs (about locking/unlocking and waiting/notifying) 
you have to set ``bool Logger::showSystemLogs`` to false or true respectively. 
This is the only setting and it has to appear above ``main`` function.

##### Private logs
If you want to display some information, you can use monitor's ``log(std::string log)`` method.


Example below
```objectivec
buffer.log("This is my log");
```
can results in:
```commandline
$ 1633 100:0:4: This is my log
```
where ``1633`` means Lamport's clock and ``100:0:4`` means unique id of monitor in distributed system.

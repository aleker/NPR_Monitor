Building and running project
----------
Current directory: *NPR_Monitor*.

####Building
```commandline
cmake
make
```
####Running application
Exmple of running application on 2 machines (cores).
```commandline
mpirun -np 2 ./bin/NPR_Monitor
```

Implementing monitor
---------
####Inherit from DistributedMonitor.
```
class ConsumerProducerQueue : public DistributedMonitor {
    // ...
};
```

####Create constructor

The only obligatory argument is implementation of `ConnectionInterface` e.g. `MPI_Connection`.
```objectivec
    ConsumerProducerQueue(std::shared_ptr<ConnectionInterface> connection) : 
    DistributedMonitor(connection) { }
```

####Override virtual functions
```objectivec
void manageReceivedData(std::string receivedData) override { 
    // ... 
}

std::string returnDataToSend() override {
    // ...
}
```
``manageReceivedData`` manages received data (in ``std::string`` form) e.g. save it to buffer.

``returnDataToSend`` transforms ready to share data into ``std::string``.

####Call `destruct()` in destructor
```objectivec
~ConsumerProducerQueue() {
    destruct();
}
```

####Creating condition variables

All new condition variables (distributed versions) must be elements of ``d_cvMap`` to work properly.
Every ``DistributedConditionVariable`` has unique name (key in ``d_cvMap``) by which it is identified.
Name is the first argument in declaration (*"id1"* in example below).

Second argument is mutex of type ``DistributedMutex`` on which condition variable will be operating.
```objectivec
d_cvMap["id1"] = std::make_shared<DistributedConditionVariable>("id1", d_mutex);
```
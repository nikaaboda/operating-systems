shared int readercount;
semaphore mutex = 1, writer = 1;
shared object data;


readers() {
    down(&mutex);
    if(++readercount == 1) {
        down(&writer);
    }
    up(&mutex);
    read_shared_object(&data);
    if(--readercount == 0) {
        up(&writer);
    }
    up(&mutex);
}

writer() {
    down(&writer);
    write_shared_object(&data) 
    up(&writer);
}
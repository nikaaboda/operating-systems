semaphore chips = N;
shared int team = 0;

runner() {
    down(&chips);
    run();
    up(&chips);
}

relay() {
    team++;
    if(team == T) {
        team = 0;
        for(int i = 0; i < T; i++) {
           down(&chips); 
        }
        for(int i = 0; i < T; i++) {
            run();   
        }
        for(int i = 0; i < T; i++) {
            up(&chips);   
        }
    }
}


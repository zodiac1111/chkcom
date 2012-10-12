#终端(arm)上运行,用于检查com口.

效率大致如下(检测com3及之后的6个com口):

```
# ./a.out 3 6
        com3    com4    com5    com6    com7    com8    
com3    -       O       O       X       X       X       
com4    O       -       O       X       X       X       
com5    O       O       -       X       X       X       
com6    X       X       X       -       X       X       
com7    X       X       X       X       -       O       
com8    X       X       X       X       O       -   
```

或者,直接检测 com3 和 com5

```
# ./a.out -d 3  5
directlink
        com3
com5    O
```

import os
import matplotlib.pyplot as plt 
import pickle


def getCUDAThreads(body):
    return max(10,i/10)
def getOpenMPThreads(body):
    if body < 100: return 2
    if body < 10000: return 5
    if body < 30000: return max(30, body / 1000)
    return 30


NBODY = [1000,4000,8000,12000,16000,20000]
# NBODY = [10000,25000,50000,75000,100000,125000,150000]
ITERATION = 1
BINARY_INF = [ 
    ['Serial','nbodySerial',[1 for i in NBODY]], 
    ['CUDA','nbodyCuda.exe',[getCUDAThreads(i) for i in NBODY]],
    # ['CUDA (Global Memory)','nbodyCuda.exe',[getCUDAThreads(i) for i in NBODY]],
    ['OpenMP','nbodyOpenMP.exe',[getOpenMPThreads(i) for i in NBODY]],
    # ['OpenMP (Double Precision)','nbodyOpenMP.exe',[getOpenMPThreads(i) for i in NBODY]],
]
SPEEDUP_REF = 'Serial'


def saveRes(file,data):
    SEPARATOR = ','

    f = open(file+'.var', 'wb')
    pickle.dump(data, f)
    f.close()

    f = open(file,'w')
    x = [i[0] for i in data]
    y = [i[1] for i in data]
    x = sorted(set(x))
    y = sorted(set(y))
    # print "X",x
    # print "Y",y
    f.write(SEPARATOR)
    for c in x:
        f.write(c)
        f.write(SEPARATOR)
    
    for r in y:
        f.write('\n')
        f.write(str(r))    
        for c in x:
            val = str(data.get((c,r),"-1"))
            f.write(SEPARATOR)
            f.write(val)
            
    f.close()

def plotTime(res,LABEL,NBODY):
    fig = plt.figure()
    fig.suptitle('Time Comparison', fontsize=20)
    for l in LABEL:
        X_AXIS = NBODY
        Y_AXIS = [res[(l,nb)][0] for nb in NBODY]        
        plt.plot(X_AXIS,Y_AXIS,marker='o')
    plt.xlabel('Number of bodies', fontsize=16)
    plt.ylabel('Time', fontsize=16)
    plt.legend(LABEL, loc='upper left')
    fig.savefig('Time.jpg')

def plotSpeedup(res,LABEL,NBODY,refLabel):
    fig = plt.figure()
    fig.suptitle('Speedup Comparison', fontsize=20)
    for l in LABEL:
        X_AXIS = NBODY
        Y_AXIS = [  res[(refLabel, nb)][0] * 1.0 / res[(l,nb)][0]  for nb in NBODY]        
        plt.plot(X_AXIS,Y_AXIS,marker='o')
    plt.xlabel('Number of bodies', fontsize=16)
    plt.ylabel('Speedup', fontsize=16)
    plt.legend(LABEL, loc='upper left')
    fig.savefig('Speedup.jpg')

def plotSpeedupEff(res,LABEL,NBODY,refLabel):
    fig = plt.figure()
    fig.suptitle('Speedup Comparison', fontsize=20)
    for l in LABEL:
        X_AXIS = NBODY
        Y_AXIS = [  res[(refLabel, nb)][0] * res[(l,nb)][0] / (res[(l,nb)][0] * res[(l,nb)][0] ) for nb in NBODY]        
        plt.plot(X_AXIS,Y_AXIS,marker='o')
    plt.xlabel('Number of bodies', fontsize=16)
    plt.ylabel('Speedup', fontsize=16)
    plt.legend(LABEL, loc='upper left')
    fig.savefig('Speedup.jpg')


def load(file):
        
    f = open(file+".var", 'rb')
    obj = pickle.load(f)
    f.close()
    return obj





res = {}

for row in BINARY_INF:
    label = row[0]
    binary = row[1]
    threadList = row[2]

    for nbody,thread in zip(NBODY, threadList):
        comm = binary + " " + str(nbody) + " " + str(ITERATION) + " " + str(thread)
        print "\n-----------\n%s (%s)" % (label,comm)
        out = os.popen(comm).read()
        print out
        time = float(out.split('Time:')[1].split(' secs')[0])
        res[(label,nbody)] = time,thread
    
LABEL = [i[0] for i in BINARY_INF]
plotTime(res,LABEL,NBODY)
plotSpeedup(res,LABEL,NBODY,SPEEDUP_REF)
saveRes('Out.csv',res)



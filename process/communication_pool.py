from multiprocessing import Pool,Manager

q=Manager().Queue()
q2=Manager().Queue()

def get1(total):
    for i in range(total):
        print('adding num %d'%(i+1,))
        q.put(i+1)

def times(num):
    while True:
        if not q.empty():
            val=q.get()
            print('times %d for the num %d'%(num,val))
            val=val*num
            q2.put(val)
        else:
            break
            
def save(filename):
    with open(filename,'w') as f:
        while True:
            if not q2.empty():
                val=q2.get()
                print('save num %d into the file %s'%(val,filename))
                f.write(str(val)+'\n')
            else:
                break

pool=Pool()
pool.apply(get1,(3,))
pool.apply(times,(10,))
pool.apply(save,('2.txt',))

pool.close()
pool.join()


while not q.empty():
    print('q:',q.get())
while not q2.empty():
    print('q2:',q2.get())

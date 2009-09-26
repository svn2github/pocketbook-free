import a

#a.initialize_module()

def proc1(a):
    print ("Hello from proc1! ", a)

def proc2(a):
    print ("Hello from proc2! ", a)

callback1 = a.Callback(proc1)
callback2 = a.Callback(proc2)

a.setCallback(callback1)
a.callCallback()
a.setCallback(callback2)
a.callCallback()

del callback1
del callback2

import subprocess
import argparse
import os

path = '.'
iterations = '10'

parser = argparse.ArgumentParser()

#parser.add_argument('--sp', action='store_true',
#                    help='run standard perf calcs with malloc')
#
#parser.add_argument('--big', action='store_true',
#                    help='run big speriment')
#
#parser.add_argument('--bvm', action='store_true',
#                    help='run big speriment')
#
#parser.add_argument('--bvmbp', action='store_true',
#                    help='run big speriment')
#
#parser.add_argument('-r', '--runs', action='store', dest='r',
#                    type=int, help='number of trials to run');

#parser.add_argument('-o', '--outfile', action='store',
#                    dest='o', help='name of output file');

pagers = ["./apager", "./dpager", "./hpager 1", "./hpager 2", "./hpager 3"]
progs = ["stack_examiner", "readmap", "lessmem", "fastmem"]
s = '_'

for pager in pagers:        
    for prog in progs:
        command = [pager, prog]
        
        filename = pager.strip("./").split()
        filename.append(prog)
        filename = s.join(filename) + ".txt"
        pathname = "./results/" + filename

        print(pathname)

        print(command)
        
        command = " ".join(command)
        
        with open( pathname, "w+") as tf:
            for i in range( 10 ):
                tf.write("------------------------------------------------------------------\n")
                tf.write("                       ITERATION %d                               \n" % (i))
                tf.write("------------------------------------------------------------------\n")
                
                print("------------------------------------------------------------------")
                print("                       ITERATION %d                               " % (i))
                print("------------------------------------------------------------------")
                p = subprocess.Popen(command, \
                                     shell=True, \
                                     cwd=os.path.dirname(os.path.realpath(__file__)), \
                                     stdout=subprocess.PIPE, \
                                     stderr=subprocess.STDOUT, \
                                     preexec_fn=os.setsid)
                
                (std,_) = p.communicate()
                    
                outstr = std.decode('latin-1')
                
                print('%d, %s' % (i, outstr))
                
                tf.write(outstr)
                
                # kill the process
                p.kill()
        


#if args['big'] == True:
#    commands = ["-o 0 -b 0 -s 1 -u 17", "-o 0 -b 1 -s 1 -u 17", "-o 0 -b 0 -s 0 -u 17", "-o 0 -b 0 -s 0 -p -u 17", "-o 0 -b 0 -s 0 -p -u 17", "-o 0 -b 0 -s 1 -m -u 17", "-o 0 -b 1 -s 1 -m -u 17", "-o 0 -b 0 -s 0 -m -u 17", "-o 0 -b 0 -s 0 -p -m -u 17", "-o 0 -b 0 -s 0 -p -m -u 17", "-o 1 -b 0 -s 1 -u 17", "-o 1 -b 1 -s 1 -u 17", "-o 1 -b 0 -s 0 -u 17", "-o 1 -b 0 -s 0 -p -u 17", "-o 1 -b 0 -s 0 -p -u 17", "-o 1 -b 0 -s 1 -m -u 17", "-o 1 -b 1 -s 1 -m -u 17", "-o 1 -b 0 -s 0 -m -u 17", "-o 1 -b 0 -s 0 -p -m -u 17", "-o 1 -b 0 -s 0 -p -m -u 17"]
#    print("num commands: %d" % (len(commands)))
#    for j in range(args['r'] or 10):        
#        with open("../outputs/big_out.txt", "a+") as tf:
#            tf.write("------------------------------------------------------------------\n")
#            tf.write("                       ITERATION %d                               \n" % (j))
#            tf.write("------------------------------------------------------------------\n")
#        print("------------------------------------------------------------------")
#        print("                       ITERATION %d                               " % (j))
#        print("------------------------------------------------------------------")
#        for i in range (len(commands)):
#            curr_l1 = []
#            curr_tlb = []
#            """ Do perf stuff """
#            command = "./speriment.o " + commands[i] 
#            p = subprocess.Popen(command, \
#                                 shell=True, \
#                                 cwd=os.path.dirname(os.path.realpath(__file__)), \
#                                 stdout=subprocess.PIPE, \
#                                 stderr=subprocess.STDOUT, \
#                                 preexec_fn=os.setsid)
#            
#            (std,_) = p.communicate()
#                
#            outstr = std.decode('utf-8')
#        
#            print('%d, %s' % (i,outstr))
#            with open("../outputs/big_out.txt", "a+") as tf:
#                tf.write(outstr)
#                
#            # kill the process
#            p.kill()
#
#if args['bvm'] == True:
#    commands = ["-o 0 -b 0 -s 1 -c 0", "-o 0 -b 1 -s 1 -c 0", "-o 0 -b 0 -s 0 -c 0", "-o 0 -b 0 -s 0 -p -c 0", "-o 0 -b 0 -s 0 -p -c 0", "-o 0 -b 0 -s 1 -m -c 0", "-o 0 -b 1 -s 1 -m -c 0", "-o 0 -b 0 -s 0 -m -c 0", "-o 0 -b 0 -s 0 -p -m -c 0", "-o 0 -b 0 -s 0 -p -m -c 0", "-o 1 -b 0 -s 1 -c 0", "-o 1 -b 1 -s 1 -c 0", "-o 1 -b 0 -s 0 -c 0", "-o 1 -b 0 -s 0 -p -c 0", "-o 1 -b 0 -s 0 -p -c 0", "-o 1 -b 0 -s 1 -m -c 0", "-o 1 -b 1 -s 1 -m -c 0", "-o 1 -b 0 -s 0 -m -c 0", "-o 1 -b 0 -s 0 -p -m -c 0", "-o 1 -b 0 -s 0 -p -m -c 0", "-o 0 -b 0 -s 1 -c 1", "-o 0 -b 1 -s 1 -c 1", "-o 0 -b 0 -s 0 -c 1", "-o 0 -b 0 -s 0 -p -c 1", "-o 0 -b 0 -s 0 -p -c 1", "-o 0 -b 0 -s 1 -m -c 1", "-o 0 -b 1 -s 1 -m -c 1", "-o 0 -b 0 -s 0 -m -c 1", "-o 0 -b 0 -s 0 -p -m -c 1", "-o 0 -b 0 -s 0 -p -m -c 1", "-o 1 -b 0 -s 1 -c 1", "-o 1 -b 1 -s 1 -c 1", "-o 1 -b 0 -s 0 -c 1", "-o 1 -b 0 -s 0 -p -c 1", "-o 1 -b 0 -s 0 -p -c 1", "-o 1 -b 0 -s 1 -m -c 1", "-o 1 -b 1 -s 1 -m -c 1", "-o 1 -b 0 -s 0 -m -c 1", "-o 1 -b 0 -s 0 -p -m -c 1", "-o 1 -b 0 -s 0 -p -m -c 1"]
#
#    print("num commands: %d" % (len(commands)))
#    for j in range(args['r'] or 5):
#        with open("../outputs/vm_try.txt", "a+") as tf:
#            tf.write("------------------------------------------------------------------\n")
#            tf.write("                       ITERATION %d                               \n" % (j))
#            tf.write("------------------------------------------------------------------\n")
#
#        print("------------------------------------------------------------------")
#        print("                       ITERATION %d                               " % (j))
#        print("------------------------------------------------------------------")
#        for i in range (len(commands)):
#            curr_l1 = []
#            curr_tlb = []
#            command = "./speriment.o " + commands[i] 
#            p = subprocess.Popen(command, \
#                                 shell=True, \
#                                 cwd=os.path.dirname(os.path.realpath(__file__)), \
#                                 stdout=subprocess.PIPE, \
#                                 stderr=subprocess.STDOUT, \
#                                 preexec_fn=os.setsid)
#            
#            (std,_) = p.communicate()
#                
#            outstr = std.decode('utf-8')
#        
#            print('%d, %s' % (i,outstr))
#            with open("../outputs/vm_try.txt", "a+") as tf:
#                tf.write(outstr)
#            
#            # kill the process
#            p.kill()
#            
#if args['bvmbp'] == True:
#    commands = ["-o 0 -b 0 -s 1 -c 0", "-o 0 -b 1 -s 1 -c 0", "-o 0 -b 0 -s 0 -c 0", "-o 0 -b 0 -s 0 -p -c 0", "-o 0 -b 0 -s 0 -p -c 0", "-o 0 -b 0 -s 1 -m -c 0", "-o 0 -b 1 -s 1 -m -c 0", "-o 0 -b 0 -s 0 -m -c 0", "-o 0 -b 0 -s 0 -p -m -c 0", "-o 0 -b 0 -s 0 -p -m -c 0", "-o 1 -b 0 -s 1 -c 0", "-o 1 -b 1 -s 1 -c 0", "-o 1 -b 0 -s 0 -c 0", "-o 1 -b 0 -s 0 -p -c 0", "-o 1 -b 0 -s 0 -p -c 0", "-o 1 -b 0 -s 1 -m -c 0", "-o 1 -b 1 -s 1 -m -c 0", "-o 1 -b 0 -s 0 -m -c 0", "-o 1 -b 0 -s 0 -p -m -c 0", "-o 1 -b 0 -s 0 -p -m -c 0", "-o 0 -b 0 -s 1 -c 1", "-o 0 -b 1 -s 1 -c 1", "-o 0 -b 0 -s 0 -c 1", "-o 0 -b 0 -s 0 -p -c 1", "-o 0 -b 0 -s 0 -p -c 1", "-o 0 -b 0 -s 1 -m -c 1", "-o 0 -b 1 -s 1 -m -c 1", "-o 0 -b 0 -s 0 -m -c 1", "-o 0 -b 0 -s 0 -p -m -c 1", "-o 0 -b 0 -s 0 -p -m -c 1", "-o 1 -b 0 -s 1 -c 1", "-o 1 -b 1 -s 1 -c 1", "-o 1 -b 0 -s 0 -c 1", "-o 1 -b 0 -s 0 -p -c 1", "-o 1 -b 0 -s 0 -p -c 1", "-o 1 -b 0 -s 1 -m -c 1", "-o 1 -b 1 -s 1 -m -c 1", "-o 1 -b 0 -s 0 -m -c 1", "-o 1 -b 0 -s 0 -p -m -c 1", "-o 1 -b 0 -s 0 -p -m -c 1"]
#
#    print("num commands: %d" % (len(commands)))
#    for j in range(args['r'] or 5):
#        with open("../outputs/vm_try_bp.txt", "a+") as tf:
#            tf.write("------------------------------------------------------------------\n")
#            tf.write("                       ITERATION %d                               \n" % (j))
#            tf.write("------------------------------------------------------------------\n")
#        print("------------------------------------------------------------------")
#        print("                       ITERATION %d                               " % (j))
#        print("------------------------------------------------------------------")
#        for i in range (len(commands)):
#            curr_l1 = []
#            curr_tlb = []
#            """ Do perf stuff """
#            command = "./speriment.o " + commands[i] 
#            p = subprocess.Popen(command, \
#                                 shell=True, \
#                                 cwd=os.path.dirname(os.path.realpath(__file__)), \
#                                 stdout=subprocess.PIPE, \
#                                 stderr=subprocess.STDOUT, \
#                                 preexec_fn=os.setsid)
#            
#            (std,_) = p.communicate()
#                
#            outstr = std.decode('utf-8')
#        
#            print('%d, %s' % (i,outstr))
#            with open("../outputs/vm_try_bp.txt", "a+") as tf:
#                tf.write(outstr)
#            
#            # kill the process
#            p.kill()
#            

import subprocess
import argparse
import os
import statistics

iterations = 20
pagers = ["./apager", "./dpager", "./hpager 1", "./hpager 2", "./hpager 3"]
progs = ["stack_examiner", "readmap", "lessmem", "fastmem"]
#pagers = ["./hpager 1"]
#progs = ["fastmem"]
s = '_'

fields_of_interest = ["utime", "stime", "ttime", "maxrss", "minflt"]

stats_dict = {}
command_order = []
for prog in progs:
    for pager in pagers:        
        command = [pager, prog]
        
        filename = pager.strip("./").split()
        filename.append(prog)
        filename = s.join(filename) + ".txt"
        pathname = "./results/" + filename

        command = " ".join(command)
        command_order.append(command)

        these_stats = {}
        for field in fields_of_interest:
            these_stats[field] = []
            
        these_stats[ "mem" ] = []
        
        with open( pathname, "w+") as tf:
            for i in range( iterations ):
                tf.write("------------------------------------------------------------------\n")
                tf.write("                       ITERATION %d                               \n" % (i))
                tf.write("------------------------------------------------------------------\n")
                
                #print("------------------------------------------------------------------")
                #print("                       ITERATION %d                               " % (i))
                #print("------------------------------------------------------------------")

                p = subprocess.Popen(command, \
                                     shell=True, \
                                     cwd=os.path.dirname(os.path.realpath(__file__)), \
                                     stdout=subprocess.PIPE, \
                                     stderr=subprocess.STDOUT, \
                                     preexec_fn=os.setsid)
                
                (std,_) = p.communicate()
                    
                outstr = std.decode('latin-1')

                # kill the process
                p.kill()
                
                # Retry when initial fixed mapping fails for additional data point
                while "Failed" in outstr:
                    #print( "Trying again" )
                    p = subprocess.Popen(command, \
                                         shell=True, \
                                         cwd=os.path.dirname(os.path.realpath(__file__)), \
                                         stdout=subprocess.PIPE, \
                                         stderr=subprocess.STDOUT, \
                                         preexec_fn=os.setsid)
                    
                    (std,_) = p.communicate()
                    
                    outstr = std.decode('latin-1')
                    
                    # kill the process
                    p.kill()

                #print("%s" % (outstr))

                # Only write the correct value for this iteration to the file
                tf.write(outstr)
                
                for line in outstr.splitlines():
                    if "mapping created at" in line:
                        continue
                    
                    if "utime" in line:
                        line = line.split(" | ")
                        val = float(line[-1])
                        
                        these_stats["utime"].append(val)
                    elif "stime" in line:
                        line = line.split(" | ")
                        val = float(line[-1])
                        
                        these_stats["stime"].append(val)
                        
                    elif "ttime" in line:
                        line = line.split(" | ")
                        val = float(line[-1])
                        
                        these_stats["ttime"].append(val)

                    elif "minflt" in line:
                        line = line.split(" | ")
                        val = float(line[-1])
                        
                        these_stats["minflt"].append(val)
                        
                    #elif "majflt" in line:
                    #    line = line.split(" | ")
                    #    val = float(line[-1])
                    #    
                    #    these_stats["majflt"].append(val)

                    elif "maxrss" in line:
                        line = line.split(" | ")
                        val = float(line[-2])
                        
                        these_stats["maxrss"].append(val)

                    elif "Mem" in line:
                        line = line.split(" !! ")
                        line = line[-1].split()
                        val = float(line[0])

                        these_stats["mem"].append(val)
            
        stats_dict[command] = these_stats

print("AVERAGES")
print("  utime\t\tstime\t\tttime\t\tmaxrss\t\tminflt\tmem")
for command in command_order:
    
    print(command)

    print("  %f\t%f\t%f\t%.2f\t\t%.2f\t%.2f"
          % (statistics.mean(stats_dict[command]["utime"]),
             statistics.mean(stats_dict[command]["stime"]),
             statistics.mean(stats_dict[command]["ttime"]),
             statistics.mean(stats_dict[command]["maxrss"]),
             statistics.mean(stats_dict[command]["minflt"]),
             #statistics.mean(stats_dict[command]["majflt"]),
             statistics.mean(stats_dict[command]["mem"])))

                    
                

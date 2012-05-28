#! /usr/bin/python

import sys

sampling_rate = 8000.0
packet_size = 128.0
packet_time = packet_size / sampling_rate

def main():
  if(len(sys.argv) != 3):
    print >> sys.stderr, "Usage: " + sys.argv[0] + " [overall drop rate] [mean burst time]"
    exit()
  drop_rate = float(sys.argv[1])
  burst_time = float(sys.argv[2])
  #model size
  print "2"
  #loss rates
  print "0.0 1.0"
  #transition probabilities
  drop_tp = 1.0/(burst_time/packet_time)
  accept_tp = (drop_rate*drop_tp)/(1.0-drop_rate)
  print str(1.0-accept_tp) + " " + str(accept_tp)
  print str(drop_tp) + " " + str(1.0-drop_tp)

if __name__ == "__main__":
  main()
  
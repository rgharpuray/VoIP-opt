#! /usr/bin/python

import sys

sampling_rate = 8000.0
packet_size = 128.0
packet_time = packet_size / sampling_rate

def main():
  if(len(sys.argv) != 2):
    print >> sys.stderr, "Usage: " + sys.argv[0] + " [lossrate]"
    exit()
  lossrate = float(sys.argv[1])
  #model size
  print "1"
  #loss rates
  print str(lossrate)
  #transition probabilities
  print "1.0"

if __name__ == "__main__":
  main()
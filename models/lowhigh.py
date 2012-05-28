#! /usr/bin/python

import sys

sampling_rate = 8000.0
packet_size = 128.0
packet_time = packet_size / sampling_rate

def main():
  if(len(sys.argv) != 5):
    print >> sys.stderr, "Usage: " + sys.argv[0] + " [low_lossrate] [high_lossrate] [low_losswindow] [high_losswindow]"
    exit()
  low_lr = float(sys.argv[1])
  high_lr = float(sys.argv[2])
  low_window = float(sys.argv[3])
  high_window = float(sys.argv[4])
  #model size
  print "2"
  #loss rates
  print str(low_lr) + " " + str(high_lr)
  #transition probabilities
  low_tp = 1.0/(low_window/packet_time)
  high_tp = 1.0/(high_window/packet_time)
  print str(1.0-low_tp) + " " + str(low_tp)
  print str(high_tp) + " " + str(1.0-high_tp)

if __name__ == "__main__":
  main()
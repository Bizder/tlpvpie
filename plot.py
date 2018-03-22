import matplotlib.pyplot as plt
import numpy as np


OUTPUTFOLDER = "c:/Users/tokodi/STUDY/ns3-container/output/"

OUTPUTFOLDER = OUTPUTFOLDER if OUTPUTFOLDER.endswith('/') else OUTPUTFOLDER + '/'

# probability
with open(OUTPUTFOLDER + 'ascii/p.bn', 'rb') as p_file:
	xs = []
	ys = []
	for line in p_file.read().splitlines():
		x, y = line.split()
		xs.append(float(x))
		ys.append(float(y))

	plt.plot(xs, ys, linewidth=1, color='r')
	plt.axis([0, xs[-1], 0, 1])
	plt.title('Drop probability')
	plt.xlabel('Time(s)')

	plt.savefig('probability.png')
	plt.close()

# delay
with open(OUTPUTFOLDER + 'ascii/delay.bn', 'rb') as delay_file:
	xs = []
	ys = []
	for line in delay_file.read().splitlines():
		x, y = line.split()
		xs.append(float(x))
		ys.append(float(y))

	plt.plot(xs, ys, linewidth=1, color='r')

	plt.plot(xs, np.array([20 for i in xrange(len(xs))]), color='b')
	plt.title('Delay')
	plt.xlabel('Time(s)')
	plt.ylabel('Delay(ms)')

	plt.savefig('delay.png')
	plt.close()

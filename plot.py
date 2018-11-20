import matplotlib.pyplot as plt
import numpy as np


OUTPUTFOLDER = "c:/Users/tokodi/OTHER/pvpie/output/"

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

	# plt.ylim(0, 500)
	plt.plot(xs, np.array([20 for i in xrange(len(xs))]), color='b')
	plt.title('Delay')
	plt.xlabel('Time(s)')
	plt.ylabel('Delay(ms)')

	plt.savefig('delay.png')
	plt.close()

# TV
with open(OUTPUTFOLDER + 'ascii/tv.bn', 'rb') as delay_file:
	xs = []
	ys = []
	for line in delay_file.read().splitlines():
		x, y = line.split()
		xs.append(float(x))
		ys.append(float(y))

	plt.plot(xs, ys, linewidth=1, color='r')
	# plt.axis([0, 30, 400000000, 500000000])

	plt.title('Threshold Value')
	plt.xlabel('Time(s)')
	plt.ylabel('TV')

	plt.savefig('tv.png')
	plt.close()

# throughput
with open(OUTPUTFOLDER + 'ascii/throughput.bn', 'rb') as delay_file:
	xs = []
	ys = []

	for line in delay_file.read().splitlines():
		x, y = line.split()
		xs.append(float(x))
		ys.append(float(y))

	# ys = [y / 1000000.0 for y in ys]

	plt.plot(xs, ys, linewidth=1, color='r')

	plt.axis([0, xs[-1], 0, 10])

	# plt.axis([0, xs[-1], 1.24, 1.25])
	plt.title('Throughput')
	plt.xlabel('Time(s)')
	plt.ylabel('Throughput(Mbps)')

	plt.savefig('throughput.png')
	plt.close()


with open(OUTPUTFOLDER + 'ascii/throughput_gold.bn', 'rb') as gold_file, open(OUTPUTFOLDER + 'ascii/throughput_silver.bn', 'rb') as silver_file, open(OUTPUTFOLDER + 'ascii/throughput_background.bn', 'rb') as background_file:
	xs = []
	ys = []
	for line in gold_file.read().splitlines():
		x, y = line.split()
		xs.append(float(x))
		ys.append(float(y))

	plt.plot(xs, ys, linewidth=1, color='y')

	xs = []
	ys = []
	for line in silver_file.read().splitlines():
		x, y = line.split()
		xs.append(float(x))
		ys.append(float(y))

	plt.plot(xs, ys, linewidth=1, color='gray')

	xs = []
	ys = []
	for line in background_file.read().splitlines():
		x, y = line.split()
		xs.append(float(x))
		ys.append(float(y))

	plt.plot(xs, ys, linewidth=1, color='b')

	plt.title('Bandwidth share')
	plt.xlabel('Time(s)')
	plt.ylabel('Throughput(Mbps)')

	plt.savefig('bws.png')
	plt.close()

# for i in range(4):
# 	with open(OUTPUTFOLDER + 'ascii/gold_dr{}.bn'.format(i), 'rb') as delay_file:
# 		xs = []
# 		ys = []

# 		for line in delay_file.read().splitlines():
# 			x, y = line.split()
# 			xs.append(float(x))
# 			ys.append(float(y))

# 		ys = [y / 1000000.0 for y in ys]

# 		plt.plot(xs, ys, linewidth=1, color='r')

# 		plt.title('Sending Rate, Gold {}'.format(i))
# 		plt.xlabel('Time(s)')
# 		plt.ylabel('DataRate(bps)')

# 		plt.savefig('gold_dr{}.png'.format(i))
# 		plt.close()

# 	with open(OUTPUTFOLDER + 'ascii/gold_pv{}.bn'.format(i), 'rb') as delay_file:
# 		xs = []
# 		ys = []

# 		for line in delay_file.read().splitlines():
# 			x, y = line.split()
# 			xs.append(float(x))
# 			ys.append(float(y))

# 		plt.plot(xs, ys, linewidth=0.1, color='r')

# 		plt.title('Pv graph, Gold {}'.format(i))
# 		plt.xlabel('Time(s)')
# 		plt.ylabel('Packet value assigned')

# 		plt.savefig('gold_pv{}.png'.format(i))
# 		plt.close()
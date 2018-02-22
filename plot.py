import matplotlib.pyplot as plt

# probability
with open('p.bn', 'rb') as p:
	xs = []
	ys = []
	for line in p.read().splitlines():
		x, y = line.split()
		xs.append(float(x))
		ys.append(float(y))

	plt.plot(xs, ys, linewidth=1, color='r')
	plt.axis([0, xs[-1], 0, 1])
	plt.title('Drop probability')
	plt.xlabel('Time(s)')
	plt.show()

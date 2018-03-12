import matplotlib.pyplot as plt
import numpy as np


xs = [pow(10, 1), pow(10, 5)]
ys = [pow(10, 6), pow(10, 2)]


plt.axis([pow(10, 3), pow(10, 5), pow(10, 2), pow(10, 6)])
plt.plot(xs, ys, linewidth=1, color='black')
# plt.title('')
plt.xlabel('Throughput (kbps)')
plt.ylabel('PV')
plt.savefig('tvf.png')


class TVF(object):

	THROUGHPUT_RATE = 'kbps'

	def TVF(self, throughput):
		pv = 520000 / 7 - (225 * throughput) / 14
		return pv if pv > 0 else 0

	def _gold(self, throughput):
		return self.TVF(throughput)

	def _silver(self, throughput):
		pv = self.TVF(throughput)
		if throughput > 1000:
			return pv / 4
		else:
			return pv / 2

	def _background(self, throughput):
		pv = self.TVF(throughput)
		if throughput > 100:
			return pv / 10
		else:
			return pv / 4

	def _voice(self, throughput):
		if throughput > 64:
			return pow(10, 5)
		else:
			return 0

	def gold(self, throughput):
		return int(self._gold(throughput))

	def silver(self, throughput):
		return int(self._silver(throughput))

	def background(self, throughput):
		return int(self._background(throughput))

	def voice(self, throughput):
		return int(self._voice(throughput))

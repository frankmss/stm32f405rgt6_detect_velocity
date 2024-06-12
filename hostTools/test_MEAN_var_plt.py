import numpy as np
import matplotlib.pyplot as plt

# 生成一个示例序列，你可以替换这里的数据为你自己的序列数据
data = np.random.normal(loc=0, scale=1, size=1000)

# 计算均值和方差
mean = np.mean(data)
variance = np.var(data)

# 绘制分布图
plt.hist(data, bins=30, density=True, alpha=0.6, color='g')
plt.axvline(mean, color='b', linestyle='dashed', linewidth=2, label=f'均值={mean:.2f}')
plt.xlabel('data')
plt.ylabel('happend')
plt.title('disturbution of data')
plt.legend()
plt.show()

print(f'mean: {mean:.2f}')
print(f'var2: {variance:.2f}')
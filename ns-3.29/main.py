# This is a sample Python script.
import os
import random

# Press Shift+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.

seeds = [
45896,
20308,
29023,
28713,
21055,
12406,
10449,
14971,
49599,
40284,
39673,
55852,
59557,
56571,
26986,
17614,
31830,
16903,
44119,
26153,
43280,
59942,
45067,
31574,
36402,
59416,
15516,
55532,
51708,
54852,
35414,
55823,
53262,
48354,
23533,
19045,
18725,
24029,
22613,
43457,
44329,
56720,
12228,
40527,
37583,
15440,
36878,
33813,
14328,
39545
]

seedIndex = 0

#读取文件，建立参数，然后运行。
fileName = './src/gpsr/model/gpsr-ptable.cc'
f = open(fileName)

c = f.read()
lineStart = c.find("int radio = ")
lineEnd = c.find(";", lineStart)

sen = c[lineStart: lineEnd]
vStart = sen.find(' = ') + 3
value = int(sen[vStart:])
f.close()

result = open('./0resultGFDR.txt', 'w')
seed = 'seed (100'

while value > 9:
    #替换参数
    f = open(fileName, 'r+')
    c = f.read()
    t = "int radio = " + str(value)
    value = value - 10
    c = c.replace(t, 'int radio = ' + str(value))
    f.close()

    print('will start' + str(value))

    f = open(fileName, 'w')
    f.write(c)
    f.close()

    result.write(str(value) + '\n')
    result.flush()

    #循环50次
    for i in range(50):
        #读取文本
        gpsr = open('./scratch/gpsr-gfdr.cc')
        test6 = gpsr.read()
        gpsr.close()

        if test6.find('seed (200)') != -1:
            break
        #替换文本
        newSeed = 'seed (' + str(seeds[seedIndex])
        seedIndex = seedIndex + 1
        test6 = test6.replace(seed, newSeed)
        seed = newSeed

        #写入文本
        gpsr = open('./scratch/gpsr-gfdr.cc', 'w')
        gpsr.write(test6)
        gpsr.close()

        #运行
        os.system('./waf --run scratch/gpsr-gfdr')
        #记录结果
        gpsrResult = open('./results/gpsr_results/pairs15/gpsr30_results.txt')
        result.write(gpsrResult.read())
        gpsrResult.close()
        result.flush()
result.close()


    #然后修改结果文件
    #os.rename('./results/gpsr_results/pairs15/gpsr30_results.txt', './results/gpsr_results/pairs15/' + str(value) + '.txt')

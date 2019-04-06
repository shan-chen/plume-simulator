import random
import sys


def usage():
    print(
        """
        usage:
        args[0]:total_nodes    
        args[1]:degree_lower
        args[2]:degree_upper
        args[3]:avg_delay
        """
    )


def gen_graph(total_nodes, degree_lower, degree_upper):
    total_links = 0
    d = {}
    for i in range(total_nodes):
        d[i] = []
    for i in range(total_nodes):
        while len(d[i]) < degree_lower:
            peer = random.randint(0, total_nodes - 1)
            if peer == i or len(d[peer]) >= degree_upper or peer in d[i]:
                continue
            d[i].append(peer)
            d[peer].append(i)
            total_links += 1
    return d, total_links


def gen_delay(d, avg_delay):
    flag = 0
    last_delay = 0
    delays = [[] for i in d.keys()]
    for i in d.keys():
        for j in d[i]:
            d[j].remove(i)
            if flag == 0:
                delay = random.randint(0, avg_delay * 2)
                flag = 1
            else:
                delay = 2 * avg_delay - last_delay
                flag = 0
            last_delay = delay
            delays[i].append([j, delay])
    return delays


def gen_file(d, delays, nodes, links):
    with open('topo.txt', 'w') as f:
        f.write(str(nodes) + ' ' + str(links) + '\n')
        for i in range(nodes):
            x = random.randint(0, 10000)
            y = random.randint(0, 10000)
            f.write(str(i) + ' ' + str(x) + ' ' + str(y) + '\n')
        for i in range(len(delays)):
            if len(delays[i]) == 0:
                continue
            for j in range(len(delays[i])):
                f.write(str(i) + ' ' + str(delays[i][j][0]) + ' ' + str(delays[i][j][1]) + ' ' + '\n')


def main():
    if len(sys.argv) != 5:
        usage()
        sys.exit()

    total_nodes = int(sys.argv[1])
    degree_lower = int(sys.argv[2])
    degree_upper = int(sys.argv[3])
    avg_delay = int(sys.argv[4])

    d, total_links = gen_graph(total_nodes, degree_lower, degree_upper)
    print(d)
    delays = gen_delay(d, avg_delay)
    print(delays)
    gen_file(d, delays, total_nodes, total_links)


if __name__ == '__main__':
    main()

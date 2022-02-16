import seaborn as sns
import pandas as pd
import matplotlib.pyplot as plt
import os

bench_result = f'{os.path.dirname(__file__)}/bench_result.txt'


def load_df():
    with open(bench_result, 'r') as f:
        lines = f.read()
    results = lines.split('\n\n')
    print(results)

    result_list = []
    i = 0
    for result in results:
        package, result_str = result.split('\n', 1)
        result_str = [v.strip() for v in result_str.split('\n')]
        result_str = [v.rsplit(':', 1) for v in result_str]
        for k, v in result_str:
            v = eval(v)
            k = k.split(':')[0]
            for v in v:
                result_list.append({'tests': k, 'package': package, 'time[s]': v})
    df = pd.DataFrame(data=result_list)
    print(df)
    plt.figure(figsize = (10,8))
    ax = sns.barplot(x="tests", y="time[s]", hue="package", data=df)
    ax.set_xticklabels(ax.get_xticklabels(), rotation=-10)
    plt.legend(fontsize='x-large', title_fontsize='40')
    plt.savefig('bench_result.png', dpi=500)


if __name__ == '__main__':
    load_df()

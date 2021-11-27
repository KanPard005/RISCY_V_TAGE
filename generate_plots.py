import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

indian_red = '#c54b6c'
steel_blue = '#5784ba'

def make_bar_plot(df, name_col, col1, col2, param, side):
    width = 0.4
    ind = np.arange(5)
    fig = plt.figure()
    ax = fig.add_subplot(111)

    rect1 = ax.bar(ind, df[col1], width, color = indian_red)
    rect2 = ax.bar(ind + width, df[col2], width, color = steel_blue)
    ax.set_ylabel('Improvement in ' + param +'(%)')
    ax.set_title('Improvement in ' + param + ' w.r.t. hashed perceptron: ' + side.title() + ' 5')
    ax.set_xticks(ind + width / 2)
    ax.set_xticklabels(df[name_col])
    ax.legend((rect1, rect2), (col1.upper(), col2.upper()))

    plt.savefig('./plots/' + side + '_5_' + param + '.png')
    plt.show()

def make_plot(df, name_col, col1, col2, param):
    print(df.head(5))
    print('Mean improvement in {} for TAGE: {:.4f}%'.format(param, df[col1].mean()))
    print('Mean improvement in {} for LTAGE: {:.4f}%'.format(param, df[col2].mean()))
    print('{} is better than {} in {} server traces'.format(col2.upper(), col1.upper(), df[df[col2] >= df[col1]].shape[0]))

    lower = min(df[col1].min(), df[col2].min())
    upper = max(df[col1].max(), df[col2].max())
    print(lower, upper)
    bins = np.linspace(start = lower, stop = upper, num = 21)
    d1, centers = np.histogram(df[col1], bins)
    d2, centers = np.histogram(df[col2], bins)
    centers = (centers[1:] + centers[:-1]) / 2
    plt.plot(centers, d1, '-', c = indian_red)
    plt.plot(centers, d2, '-', c = steel_blue)
    plt.title('Frequency plot of percentage improvement in ' + param)
    plt.xlabel('Percentage improvement')
    plt.ylabel('Frequency')
    plt.legend([col1.upper(), col2.upper()])
    plt.savefig('./plots/' + param + '_histogram.png')
    plt.show()

    df = df.sort_values([col2, col1], ascending = False)
    make_bar_plot(df.head(5), name_col, col1, col2, param, 'top')
    make_bar_plot(df.tail(5), name_col, col1, col2, param, 'bottom')



### Plots for accuracy
acc_df = pd.read_csv('./results/accuracy.csv')

acc_df['ltage'] -= acc_df['hashed_perceptron']
acc_df['tage'] -= acc_df['hashed_perceptron']

make_plot(acc_df, 'Unnamed: 0', 'tage', 'ltage', 'accuracy')

### Plots for MPKI
mpki_df = pd.read_csv('./results/mpki.csv')
mpki_df['ltage'] = - (mpki_df['ltage'] - mpki_df['hashed_perceptron']) * 100 / mpki_df['hashed_perceptron']
mpki_df['tage'] = - (mpki_df['tage'] - mpki_df['hashed_perceptron']) * 100 / mpki_df['hashed_perceptron']

make_plot(mpki_df, 'Unnamed: 0', 'tage', 'ltage', 'MPKI')
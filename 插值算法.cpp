
#include <stdio.h>
#include <math.h>
#include <stdlib.h> 

double lagrange(double x, double x_points[], double y_points[], int n) {
    double result = 0.0;
    for (int i = 0; i < n; i++) {
        double term = y_points[i];
        for (int j = 0; j < n; j++) {
            if (j != i) {
                term *= (x - x_points[j]) / (x_points[i] - x_points[j]);
            }
        }
        result += term;
    }
    return result;
}

double newton(double x, double x_points[], double y_points[], int n) {
    double *f = (double*)malloc(n * sizeof(double));
    for (int i = 0; i < n; i++) f[i] = y_points[i];
    for (int k = 1; k < n; k++) {
        for (int i = n-1; i >= k; i--) {
            f[i] = (f[i] - f[i-1]) / (x_points[i] - x_points[i-k]);
        }
    }
  
    double result = f[0];
    double term = 1.0;
    for (int i = 1; i < n; i++) {
        term *= (x - x_points[i-1]);
        result += f[i] * term;
    }
    free(f);
    return result;
}

double piecewise_linear(double x, double x_points[], double y_points[], int n) {

    if (x <= x_points[0]) return y_points[0];
    if (x >= x_points[n-1]) return y_points[n-1];
    int i = 0;
    while (x > x_points[i+1] && i < n-2) i++;
    double t = (x - x_points[i]) / (x_points[i+1] - x_points[i]);
    return (1-t) * y_points[i] + t * y_points[i+1];
}

void generate_plot_data(double x_start, double x_end, int steps,
                        double x_nodes[], double y_nodes[], int n_nodes) {
    FILE *fp = fopen("interp_data.txt", "w");
    if (!fp) { perror("文件打开失败"); return; }
    double step = (x_end - x_start) / steps;
    for (int i = 0; i <= steps; i++) {
        double x = x_start + i * step;
        double y_true = exp(-x);
        double y_lag1 = lagrange(x, x_nodes, y_nodes, 2);     
        double y_lag2 = lagrange(x, x_nodes, y_nodes, 3);    
        double y_new2 = newton(x, x_nodes, y_nodes, 3);
        double y_linear = piecewise_linear(x, x_nodes, y_nodes, n_nodes);
        fprintf(fp, "%f %f %f %f %f %f\n",
                x, y_true, y_lag1, y_lag2, y_new2, y_linear);
    }
    fclose(fp);

}

int main() {
  
    double x_nodes[] = {1.0, 2.0, 3.0};
    double y_nodes[] = {0.367879441, 0.135335283, 0.049787068};
    int n = sizeof(x_nodes) / sizeof(x_nodes[0]);
    double x_target = 2.6;
    double true_val = exp(-x_target);

    printf("==================================================\n");
    printf("插值结果对比 (x = %.1f)\n", x_target);
    printf("真实值 e^(-%.1f) = %.10f\n", x_target, true_val);
    printf("--------------------------------------------------\n");

  
    double x_near[] = {2.0, 3.0};
    double y_near[] = {0.135335283, 0.049787068};
    double lag1 = lagrange(x_target, x_near, y_near, 2);
  
    double lag2 = lagrange(x_target, x_nodes, y_nodes, 3);
    printf("拉格朗日插值:\n");
    printf("  1次插值 (节点2,3): %.10f, 绝对误差 = %.2e\n", lag1, fabs(lag1 - true_val));
    printf("  2次插值 (节点1,2,3): %.10f, 绝对误差 = %.2e\n", lag2, fabs(lag2 - true_val));


    double new1 = newton(x_target, x_near, y_near, 2);
    double new2 = newton(x_target, x_nodes, y_nodes, 3);
    printf("牛顿插值:\n");
    printf("  1次插值 (节点2,3): %.10f, 绝对误差 = %.2e\n", new1, fabs(new1 - true_val));
    printf("  2次插值 (节点1,2,3): %.10f, 绝对误差 = %.2e\n", new2, fabs(new2 - true_val));

    double linear_val = piecewise_linear(x_target, x_nodes, y_nodes, n);
    printf("分段线性插值:\n");
    printf("  插值结果: %.10f, 绝对误差 = %.2e\n", linear_val, fabs(linear_val - true_val));

 

    return 0;
}

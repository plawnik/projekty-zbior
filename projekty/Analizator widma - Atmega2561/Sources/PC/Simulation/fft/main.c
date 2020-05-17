#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#define N 8192
#ifndef PI
#define PI (float)3.14159265
#endif

#define RANDOM
#define BENCHMARK

float signal[N];
float Xre[N];
float Xim[N];
float Xmag[N];

void complex_fft(float rex[], float imx[], int k);


int main()
{
    srand(time(0));
    for (int n = 0; n < N; ++n) {
        #ifdef RANDOM
        // random value from 0 to 1
        Xre[n] = ((2.0 * rand()) / RAND_MAX) - 1.0;
        signal[n]=Xre[n];
        #else
        Xre[n] = (float)5*(cos(2 * 3.14 * n / N))+2+3*cos(10 * 3.14 * n / N);
        signal[n]=Xre[n];
        #endif
    }


    complex_fft(Xre, Xim, N);





      // save result to scilab
    FILE *plots = fopen("plots.sci","w");
    // clear commands
    fprintf(plots, "//clear commands\n");
    fprintf(plots, "clear;\n");
    fprintf(plots, "xdel();\n");
    fprintf(plots, "clc;\n");
    fprintf(plots, "clf();\n");
    // write vectors
    fprintf(plots, "//write vectors\n");
    fprintf(plots, "x=[");
    for(int i=0;i<N;i++) fprintf(plots,"%f ",signal[i]);
    fprintf(plots, "];\x0A");
    fprintf(plots, "Xre=[");
    for(int i=0;i<=((N/2));i++) fprintf(plots,"%f ",Xre[i]);
    fprintf(plots, "];\x0A");
    fprintf(plots, "Xim=[");
    for(int i=0;i<=((N/2));i++) fprintf(plots,"%f ",Xim[i]);
    fprintf(plots, "];\x0A");
    fprintf(plots, "Xmag=[");
    for(int i=0;i<=((N/2));i++) fprintf(plots,"%f ",Xmag[i]);
    fprintf(plots, "];\x0A");
    // draw first plot
    fprintf(plots, "//draw first plot\n");
    fprintf(plots, "xmin=min(x);\n");
    fprintf(plots, "xmax=max(x);\n");
    fprintf(plots, "ymax=length(x);\n");
    fprintf(plots, "n = [0:ymax-1];\n");//N-1
    fprintf(plots, "subplot(3,1,1);\n");
    fprintf(plots, "plot2d([0:(ymax-1)],x,rect=[0,xmin,ymax-1,xmax]);\n");
    fprintf(plots, "legend(\"Input signal\");\n");
    fprintf(plots, "xtitle(\"Oryginal input signal\",\"x(n)\",\"Amplitude\");\n");
    // second plot
    fprintf(plots, "//second plot\n");
    fprintf(plots, "all = [Xre,Xim];\n");
    fprintf(plots, "xmax=max(all);\n");
    fprintf(plots, "xmin=min(all);\n");
    fprintf(plots, "ymax=length(Xre);\n");
    fprintf(plots, "zero = zeros(ymax,1);\n");
    fprintf(plots, "subplot(3,1,2);\n");
    fprintf(plots, "x_x=[0:1:(ymax-1)];\n");
    fprintf(plots, "xtitle(\"Xre and Xim amplitudes\",\"Xre(n),Xim(n)\",\"Amplitude\");\n");
    fprintf(plots, "plot2d(x_x,Xre,rect=[0,xmin,ymax,xmax]);\n");
    fprintf(plots, "plot2d(x_x,Xim);\n");
    fprintf(plots, "h=gcf();\n");
    fprintf(plots, "h.children.children(1).children.line_style = 8;\n");
    fprintf(plots, "h.children.children(1).children.thickness = 1.5;\n");
    fprintf(plots, "legend(\"Xre\", \"Xim\");\n");
    fprintf(plots, "plot2d(x_x,zero);\n");
    fprintf(plots, "clear h;\n");
    fprintf(plots, "clear all;\n");
    // third plot
    fprintf(plots, "//third plot\n");
    fprintf(plots, "subplot(3,1,3)\n");
    fprintf(plots, "bar([0:ymax-1],Xmag);\n");
    fprintf(plots, "xtitle(\"DFT result\",\"X(n)\",\"Amplitude\");\n");
    fprintf(plots, "xgrid(1);\n");
    fprintf(plots, "legend(\"n-harmonic\")\n");
    fprintf(plots, "xmax=max(Xmag);\n");
    fprintf(plots, "ymax=length(Xmag);\n");
    fprintf(plots, "h = gca();\n");
    fprintf(plots, "h.data_bounds = [-0.2,0 ; ymax, xmax+0.05];\n");
    fprintf(plots, "h.tight_limits=[\"on\",\"on\"];\n");
    fprintf(plots, "h.auto_ticks=[\"on\",\"on\",\"on\"];");
    fprintf(plots, "clear h;\n");
    // set plot window size
    fprintf(plots, "//set window size\n");
    fprintf(plots, "f = get(\"current_figure\");\n");
    fprintf(plots, "f.figure_position = [0,0];\n");
    fprintf(plots, "f.figure_size = [1000,1000];\n");
    fclose(plots);

    #ifndef BENCHMARK
    printf("FLOATING POINT\n");
    for (int i = 0; i < (N); i++) {
        printf("Nr = %d, Xre = %f, Xim = %f, Xmag = %f\n\r", i, Xre[i], Xim[i], Xmag[i]);
    }
    #endif
}




void complex_fft(float rex[], float imx[], int k)
{
    int nm1 = k - 1; // max probes index
    int nd2 = k / 2; // number of probes/2
    int m = (log(k) / log(2)); // number of probes bits
    float tr, ti; // temporary variables
    int j = nd2; // variable to destroy in calc
    // reverse bit sorting
    for (int i = 1; i <= (k - 2); i++) {
        if (i < j) {
            tr = rex[j];
            ti = imx[j];
            rex[j] = rex[i];
            imx[j] = imx[i];
            rex[i] = tr;
            imx[i] = ti;
        }
        int k = nd2;
        while (k <= j) {
            j = j - k;
            k = k / 2;
        }
        j = j + k;
    }

    // calc complex fft
    for (int l = 1; l <= m; l++) {
        int le = pow(2, l); // dft width, block offset
        int le2 = le / 2; // W=cos(2*pi/2^l)-jsin(2*pi/2^l)
        float ur = 1; // start value of W_real
        float ui = 0; // star value of W_imag
        float sr = cos(PI / le2); //base of fourier real
        float si = -sin(PI / le2); //base of fourier imag
        for (int j = 1; j <= le2; j++) { // next blocks
            int jm1 = j - 1;
            for (int i = jm1; i <= nm1; i = i + le) { // next butterflies
                int ip = i + le2; // index of lower probe
                tr = rex[ip] * ur - imx[ip] * ui; // butterfly calc real part
                ti = rex[ip] * ui + imx[ip] * ur; // butterfly calc imag part
                rex[ip] = rex[i] - tr; // correct value
                imx[ip] = imx[i] - ti; // correct value
                rex[i] = rex[i] + tr; // calc final probe val
                imx[i] = imx[i] + ti; // calc final probe val
            }
            tr = ur; // copy to temp variable
            ur = tr * sr - ui * si; // calculate new W_real value
            ui = tr * si + ui * sr; // calcualte new W_imag value
        }
    }

    // find real harmonic amplitudes
    for (int i = 0; i < k; i++) {
        if (i == 0 || i == k / 2) {
            rex[i] /= k;
            imx[i] /= k;
        }
        else {
            rex[i] /= k / 2;
            imx[i] /= k / 2;
        }
        Xmag[i] = sqrt(rex[i] * rex[i] + imx[i] * imx[i]);
    }
}

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>


#define N 8192
#define BENCHMARK

#define RANDOM


void dft(float input[], float rex[], float imx[], float pwr[], int k);

float signal[N];
float Xre[(N/2)+1];
float Xim[(N/2)+1];
float Xmag[(N/2)+1];



int main()
{
    srand(time(0));
    for(int n=0 ; n<N ; ++n){
        #ifdef RANDOM
        // random value from 0 to 1
        signal[n] = ((2.0 * rand()) / RAND_MAX) - 1.0;
        //signal[n]+=0.5; // add 0 harmonic
        #else
        signal[n] = (float)30*cos(2*3.14*n/N)+10*sin(8*3.14*n/N);
        signal[n]+=5; // add 0 harmonic
        #endif
    }

    dft(signal,Xre,Xim,Xmag,N);

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
    // console debug
    for(int i=0;i<(N/2+1);i++){
        printf("x = %f, X = %f\n\r",signal[i],Xmag[i]);
    }
    #endif
}



//********************************************************************
// input[]-input vect; rex[]-re part of result; imx- im part of result
// pwr[]-magnitude of of result; k-size of input vector
//********************************************************************
void dft(float input[], float rex[], float imx[], float pwr[],int k){
#ifndef PI
#define PI (float)3.14159265
#endif
    for(int i=0;i<=(k/2);i++){
        rex[i]=0;imx[i]=0; //clear
        for(int j=0;j<k;j++){
            // calc real part value
            rex[i] +=input[j]*cos(2*PI*j*i/k);
            // calc imag part value
            imx[i] +=input[j]*sin(2*PI*j*i/k);
        }
        // form value
        if(i==0||i==k/2){
            rex[i]/=k; imx[i]/=k;
        }else{
            rex[i]/=k/2; imx[i]/=k/2;
        }
		// calc magnitude
        pwr[i]=sqrt(rex[i]*rex[i]+imx[i]*imx[i]);
    }
}

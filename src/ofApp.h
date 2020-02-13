#pragma once

#include "ofMain.h"
#include "ofxFaceTracker.h"
#include "ofxCv.h"
#include <mysql/mysql.h>
#include <math.h>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <mysql/mysql.h>
#include <string.h>
#include<iostream>
#include<fstream>

#define DBHOST "localhost"
#define DBPORT 3306
#define DBUSER "root"
#define DBPASS "ryo167497"
#define DBNAME "test"


class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
        float changeEmo05(float z_score);
        float changeEmo075(float z_score);
        float changeEmo0125(float z_score);
        float changeEmo025(float z_score);
        float changeEmo045(float z_score);
        float changeEmo0375(float z_score);
        float changeEmo1(float z_score);
    
        void transform();
        void normal();
        void joy();
        void disgust();
        void sql(int i);
        void baseline(int i);
        double stdev(int *data, int n);
        
        ofVideoGrabber cam;
                
        int ww; //window width
        int wh; //window height
        int cw; //camera width
        int ch; //camera height
        
        int s;
        int m;
        int start_t;
        int end_t;
        int limit;
        int temp;
        
        int mode;   //display mode
        int wmode;   //wireframe mode
        int face[8];  //random 8
        
        ofxFaceTracker tracker;
        ofVec2f position;
        float scale;
        ofVec3f orientation;
        ofMatrix4x4 rotationMatrix;
        cv::Mat translation, rotation;
        ofMatrix4x4 pose;
        ofMesh camMesh;
        
        float fpx[66];
        float fpy[66];
        float fpz[66];
        
        float addx[66];
        float addy[66];
        float addz[66];

        const char *query;
        std::string que = "SELECT rri1, rri2, rri3 FROM yoneda6 WHERE count = ";
        std::string ry;
        int r;
        int i = 0;
        int sum = 0;
        int average;
        int bl_data[2000];
        int data[2000];
        //int comarison[];
        int count = 0;
        double sd;
        float z_score;
        float Max = 150;
    
        bool flag;
        double hrv;
};

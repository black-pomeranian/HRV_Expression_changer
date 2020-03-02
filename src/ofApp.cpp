#include "ofApp.h"

MYSQL_RES *resp;
MYSQL_ROW row;
//connector
MYSQL *conn;
MYSQL_STMT *stmt;

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetVerticalSync(true); //フレーム数と描画回数を同期
    ofSetDrawBitmapMode(OF_BITMAPMODE_MODEL_BILLBOARD); //ビットマップ描画モード採用
    //ofSetFrameRate(10); //フレームレート設定
    ofEnableAlphaBlending(); //透明度設定
    
    //window size
    ww = 1280;
    wh = 720;
    ofSetWindowShape(ww, wh);
    
    //camera size;
    cw = ww;
    ch = wh;
    cam.initGrabber(cw,ch);
    
    //setup - FaceTracker
    tracker.setup();
    
    mode = 0;
    wmode = 0;
    normal();
        
}

//--------------------------------------------------------------
void ofApp::update(){
    

    //カメラ更新
    cam.update();
    //カメラが動いていたらトラッキング更新
    if(cam.isFrameNew()){
        tracker.update(ofxCv::toCv(cam));
        //検出された顔の位置，大きさ，傾き，
        position = tracker.getPosition();
        scale = tracker.getScale();
        orientation = tracker.getOrientation();
        rotationMatrix = tracker.getRotationMatrix();
        
    }
    
    //現在の秒，分を取得
    s = ofGetSeconds();
    m = ofGetMinutes();
    
    //HRVのログ
    ofstream ofs("/Users/yonedaryou/Downloads/OF_ROOT/apps/myApps/Expression_Change-myBeat/bin/data/hrv_Log.csv", std::ios::app);
    
    //std::printf("%d ", i);

    if(i < 10){
        baseline(i);
        if(flag){
            ++i;
            flag = false;
        }
    }
    
    if(10 == i){
        average = sum/30;
        sd = stdev(bl_data, 30);
        std::printf("%lf ", sd);
    }
    if(10 <= i){
        sql(i);
         hrv = stdev(data, 3);
        std::printf("%lf ", hrv);
        z_score = hrv - sd;  //標準より少ない方がリラックス(正になる)
        ofs << hrv << ",";
        
        if(flag){
            ++i;
            flag = false;
        }
        
    }
    
    //++i;
}

    

//--------------------------------------------------------------
void ofApp::draw(){
    
    ofBackground(255,255,255,50); //背景色
    ofSetColor(255);
    cam.draw(0,0); //ofVideoGrabberで動画を描画
    //gui.draw();
    ofSetColor(0,255,0);
    
    ofDrawBitmapString(ofToString((int ) mode ) , 200,200); //画面に表情モードの文字描画
    
    //円のランダム生成
    //if(ofGetFrameNum() % 100==0){
    if(ofRandom(0, 1000) > 990 && ellipse_count == 0){

        //円の位置
        location_x = ofRandom(ofGetWindowWidth());
        location_y = ofRandom(ofGetWindowHeight());
        
        ofSetColor(186, 85, 211);
        ofFill();
        ofDrawEllipse(location_x, location_y, 100, 100);

        ellipse_count += 1;
    }
    else if (ellipse_count > 0){
        
        //円の描写
        ofSetColor(186, 85, 211);
        ofFill();
        ofDrawEllipse(location_x, location_y, 100, 100);

        location_x = 0;
        location_y = 0;
        ellipse_count = 0;
        
    }
    
    
    
    
    if(mode>1){
        mode = 0;
        wmode = 1;
    }
    
    //顔を検出したら
    if(tracker.getFound()){
        
        ofSetColor(255);
        //ofSetupScreenOrtho(ww, wh, OF_ORIENTATION_UNKNOWN, true, -1000, 1000);//????
        ofSetupScreenOrtho(ww, wh, -1000, 1000);//????

        ofSetOrientation(OF_ORIENTATION_DEFAULT, true);
        //cam
        //ofPopMatrixとセットで座標系の変化をこのブロック内に
        ofPushMatrix();
        //顔の位置を座標の中心に
        ofTranslate(position.x ,position.y );
        //座標系をscale倍(画面と顔の距離を合わせる？)
        ofScale(scale,scale,scale);
        //引数と現在の行列を乗算
        ofxCv::applyMatrix(rotationMatrix);
        ofPopMatrix();
        
        //メッシュを取得
        camMesh = tracker.getMeanObjectMesh();
        
        ofPushMatrix();
        //上述
        ofTranslate( position.x , position.y );
        ofSetColor(255,255,255);
        ofScale(scale, scale, scale);
        
        //transform
        //特長点の現在位置取得
        for( int i = 0 ; i < 66 ; i++ ){
            fpx[i] = tracker.getObjectPoint(i).x;
            fpy[i] = tracker.getObjectPoint(i).y;
            fpz[i] = tracker.getObjectPoint(i).z;
        }
        //モードによってメソッド切り替え
        switch (mode) {
            case 0:
                normal();
                break;
            case 1:
                if(z_score > 0){
                    joy();
                    break;
                }
                else if (z_score < 0){
                    disgust();
                    break;
                }
        }
        
        ofSetColor(255);
        
        //表情変形
        transform();
        
        //画面のx軸，y軸，z軸をfloatで45度回転
        ofRotateXDeg(orientation.x * 45.0f);
        ofRotateYDeg(orientation.y * 45.0f);
        ofRotateZDeg(orientation.z * 45.0f);
        
        
        ofPopMatrix();
        cam.getTexture().bind();
        ofSetColor(255);
        cam.getTexture().unbind();
        ofPopMatrix();
        
        //mapping
        ofPushMatrix();
        ofTranslate( position.x , position.y );
        ofSetColor(255,255,255);
        ofScale(scale, scale, scale);
        ofRotateXDeg(orientation.x * 45.0f);
        ofRotateYDeg(orientation.y * 45.0f);
        ofRotateZDeg(orientation.z * 45.0f);
        cam.getTexture().bind();
        camMesh.draw();
        cam.getTexture().unbind();
        
        ofPopMatrix();
        
        
        if(wmode==1){
            ofPushMatrix();
            ofTranslate( position.x , position.y );
            ofSetColor(255,255,255);
            ofScale(scale, scale, scale);
            ofRotateXDeg(orientation.x * 45.0f);
            ofRotateYDeg(orientation.y * 45.0f);
            ofRotateZDeg(orientation.z * 45.0f);
            cam.getTexture().bind();
            camMesh.drawWireframe();
            cam.getTexture().unbind();
            ofPopMatrix();
        }
    }
    
    //表情のログ
     ofstream ofs("/Users/yonedaryou/Downloads/OF_ROOT/apps/myApps/Expression_Change-myBeat/bin/data/face_Log.csv", std::ios::app);
    for( int i = 0 ; i < 66 ; i++ ){
        ofs << tracker.getObjectPoint(i).x << ",";
        ofs << tracker.getObjectPoint(i).y << ",";
        ofs << tracker.getObjectPoint(i).z << ",";
    }
     
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    if(key==OF_KEY_RETURN){
        mode++;
    }
    
    if(key == 'r') {
        tracker.reset();
    }
}



void ofApp::transform(){
    ofBeginShape(); //ofEndShapeとセットでポリゴン表示
    
    //それぞれの特徴点の現在位置に表情要素を足す
    for( int i = 0 ; i < 66 ; i++ ){
        camMesh.setVertex(i, ofPoint(fpx[i]+addx[i],fpy[i]+addy[i],fpz[i]+addz[i]));
    }
    
    ofEndShape();
}

void ofApp::normal(){
    for( int i = 0 ; i < 66 ; i++ ){
        addx[i] = 0;
        addy[i] = 0;
        addz[i] = 0;
    }
}

float ofApp::changeEmo05(float z_score){
    return z_score * 0.5/(Max - sd);
}

float ofApp::changeEmo075(float z_score){
    return z_score * 0.75/(Max - sd);
    std::printf("change");

}

float ofApp::changeEmo0125(float z_score){
    return z_score * 0.125/(Max - sd);
}

float ofApp::changeEmo025(float z_score){
    return z_score * 0.25/(Max - sd);
}

float ofApp::changeEmo045(float z_score){
    return z_score * 0.45/(Max - sd);
}

float ofApp::changeEmo0375(float z_score){
    return z_score * 0.375/(Max - sd);
}

float ofApp::changeEmo1(float z_score){
    return z_score * 1/(Max - sd);
}


void ofApp::baseline(int i){
    //RRIのログ
    ofstream ofs("/Users/yonedaryou/Downloads/OF_ROOT/apps/myApps/Expression_Change-myBeat/bin/data/rri_Log.csv", std::ios::app);
    
    char sql_str[255];
    
    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, DBHOST, DBUSER, DBPASS, DBNAME, DBPORT, NULL, 0)) {
        std::exit(1);
    }
    
    //ry = (int)i + '0';
    ry = std::to_string(i);
    std::string query_str = que + ry;
    query = query_str.c_str();
    
    std::snprintf( &sql_str[0] , sizeof(sql_str)-1 , query );
    if( mysql_query( conn , &sql_str[0] ) ){
      // error
      mysql_close(conn);
      std::exit(-1);
    }
    
    resp = mysql_use_result(conn);
    while((row = mysql_fetch_row(resp)) != NULL ){
      unsigned int col;
      for(col = 0; col < mysql_num_fields(resp); col++){
          sum = sum + atoi(row[col]);
          bl_data[count] = atoi(row[col]);
          
          ofs << row[col] << ",";
          //std::printf("%d ", i);
          
          /*if( comarison[col] != atoi(row[col]) ){
              
          }*/

          ++count;

      }
          flag = true;
    }
    
    mysql_free_result(resp);
    mysql_close(conn);
    


    
}

void ofApp::sql(int i){
    //RRIのログ
    ofstream ofs("/Users/yonedaryou/Downloads/OF_ROOT/apps/myApps/Expression_Change-myBeat/bin/data/rri_Log.csv", std::ios::app);

    
    char sql_str[255];
    int count = 0;
    
    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, DBHOST, DBUSER, DBPASS, DBNAME, DBPORT, NULL, 0)) {
        std::exit(1);
    }
    
    ry = std::to_string(i);
    std::string query_str = que + ry;
    query = query_str.c_str();
    
    std::snprintf( &sql_str[0] , sizeof(sql_str)-1 , query );
    if( mysql_query( conn , &sql_str[0] ) ){
      // error
      mysql_close(conn);
      std::exit(-1);
    }
    
    resp = mysql_use_result(conn);
    while((row = mysql_fetch_row(resp)) != NULL ){
      unsigned int col;
      for(col = 0; col < mysql_num_fields(resp); col++){
          sum = sum + atoi(row[col]);
          data[count] = atoi(row[col]);
          ofs << row[col] << ",";
          //std::printf("%s ", row[col]);
          //std::printf("%d ", i);

          ++count;
      }
            flag = true;
    }
    
        
    mysql_free_result(resp);
    mysql_close(conn);
    
}


double ofApp::stdev(int *data, int n) {
  int sum = 0;
  double mean;
  double var = 0.0;
  int i;

  for (i = 0; i < n; i++) {
    sum += data[i];
  }
  mean = (double)sum / n;

  for (i = 0; i < n; i++) {
    var += (data[i] - mean) * (data[i] - mean);
  }
  return sqrt(var / (n - 1));
}



/*****正と負それぞれの値に変換するメソッドを作る！***********/
void ofApp::joy(){                                     // joy - add - disgust

    //右眉:17-21
    addx[17] = 0.0000; addy[17] = -changeEmo05(z_score); addz[17] = 0.0000; //-0.5 ~ addy ~ 0
    addx[18] = 0.0000; addy[18] = -changeEmo075(z_score); addz[18] = 0.0000; //-0.75 ~ addy ~ -0.125
    addx[19] = 0.0000; addy[19] = -changeEmo075(z_score); addz[19] = 0.0000; //-0.75 ~ addy ~ -0.25
    addx[20] = 0.0000; addy[20] = -changeEmo075(z_score); addz[20] = 0.0000; //-0.75 ~ addy ~ -0.25
    addx[21] = 0.0000; addy[21] = -changeEmo05(z_score); addz[21] = 0.0000; //-0.5 ~ addy ~ -0.5
    //左眉:22-26
    addx[22] = 0.0000; addy[22] = -changeEmo05(z_score); addz[22] = 0.0000; // -0.5 ~ addy ~ -0.5
    addx[23] = 0.0000; addy[23] = -changeEmo075(z_score); addz[23] = 0.0000; // -0.75 ~ addy ~ -0.25
    addx[24] = 0.0000; addy[24] = -changeEmo075(z_score); addz[24] = 0.0000; // -0.75 ~ addy ~ -0.25
    addx[25] = 0.0000; addy[25] = -changeEmo075(z_score); addz[25] = 0.0000; // -0.75 ~ addy ~ -0.125
    addx[26] = 0.0000; addy[26] = -changeEmo05(z_score); addz[26] = 0.0000; // -0.5 ~ addy ~ 0
    
    //鼻筋:27-30
    addx[27] = 0.0000; addy[27] = 0.0000; addz[27] = 0.0000;
    addx[28] = 0.0000; addy[28] = 0.0000; addz[28] = 0.0000;
    addx[29] = 0.0000; addy[29] = 0.0000; addz[29] = 0.0000;
    addx[30] = 0.0000; addy[30] = 0.0000; addz[30] = 0.0000;
    //鼻底:31-35
    addx[31] = 0.0000; addy[31] = 0.0000; addz[31] = 0.0000;
    addx[32] = 0.0000; addy[32] = 0.0000; addz[32] = 0.0000;
    addx[33] = 0.0000; addy[33] = 0.0000; addz[33] = 0.0000;
    addx[34] = 0.0000; addy[34] = 0.0000; addz[34] = 0.0000;
    addx[35] = 0.0000; addy[35] = 0.0000; addz[35] = 0.0000;
    
    //右目:36-41
    addx[36] = -changeEmo025(z_score); addy[36] = -changeEmo0125(z_score); addz[36] = 0.0000; // -0.25 ~ addx ~ -0.25, -0.125 ~ addy ~ 0.125
    addx[37] = 0.0000; addy[37] = -changeEmo025(z_score); addz[37] = 0.0000; //-0.450 ~ addy ~ 0.250
    addx[38] = 0.0000; addy[38] = -changeEmo025(z_score); addz[38] = 0.0000; //-0.450 ~ addy ~ 0.250
    addx[39] = changeEmo025(z_score); addy[39] = -changeEmo0125(z_score); addz[39] = 0.0000; //0.25 ~ addx ~ 0.25, -0.1250 ~ addy ~ 0.1250
    addx[40] = 0.0000; addy[40] = 0.0000; addz[40] = 0.0000;
    addx[41] = 0.0000; addy[41] = 0.0000; addz[41] = 0.0000;
    //左目:42-47
    addx[42] = -changeEmo025(z_score); addy[42] = -changeEmo0125(z_score); addz[42] = 0.0000; // -0.25 ~ addx ~ -0.25, -0.125 ~ addy ~ 0.125
    addx[43] = 0.0000; addy[43] = 0.0000; addz[43] = 0.0000; //-0.450 ~ addy ~ 0.250
    addx[44] = 0.0000; addy[44] = 0.0000; addz[44] = 0.0000; //-0.450 ~ addy ~ 0.250
    addx[45] = changeEmo025(z_score); addy[45] = -changeEmo0125(z_score); addz[45] = 0.0000; //0.25 ~ addx ~ 0.25, -0.1250 ~ addy ~ 0.1250
    addx[46] = 0.0000; addy[46] = 0.0000; addz[46] = 0.0000;
    addx[47] = 0.0000; addy[47] = 0.0000; addz[47] = 0.0000;
    
    //口:48-65
    //左端
    addx[48] = -changeEmo1(z_score); addy[48] = -changeEmo1(z_score); addz[48] = 0.0000; //-1 ~ addx ~ -0.125, -1 ~ addy ~ 0.375
    
    //右端
    addx[54] = changeEmo1(z_score); addy[54] = -changeEmo1(z_score); addz[54] = 0.0000; //-1 ~ addx ~ -0.125, -1 ~ addy ~ 0.375
    
    //上唇上側
    addx[49] = 0.0000; addy[49] = 0.0000; addz[49] = 0.0000;
    addx[50] = 0.0000; addy[50] = 0.0000; addz[50] = 0.0000;
    addx[51] = 0.0000; addy[51] = 0.0000; addz[51] = 0.0000;
    addx[52] = 0.0000; addy[52] = 0.0000; addz[52] = 0.0000;
    addx[53] = 0.0000; addy[53] = 0.0000; addz[53] = 0.0000;
    //上唇下側
    addx[60] = 0.0000; addy[60] = 0.0000; addz[60] = 0.0000;
    addx[61] = 0.0000; addy[61] = 0.0000; addz[61] = 0.0000;
    addx[62] = 0.0000; addy[62] = 0.0000; addz[62] = 0.0000;
    
    //下唇上側
    addx[63] = 0.0000; addy[63] = 0.0000; addz[63] = 0.0000;
    addx[64] = 0.0000; addy[64] = 0.0000; addz[64] = 0.0000;
    addx[65] = 0.0000; addy[65] = 0.0000; addz[65] = 0.0000;
    //下唇下側
    addx[55] = 0.0000; addy[55] = 0; addz[55] = 0.0000;
    addx[56] = 0.0000; addy[56] = 0; addz[56] = 0.0000;
    addx[57] = 0.0000; addy[57] = 0; addz[57] = 0.0000;
    addx[58] = 0.0000; addy[58] = 0; addz[58] = 0.0000;
    addx[59] = 0.0000; addy[59] = 0; addz[59] = 0.0000;
    
    
}

void ofApp::disgust(){                                     // joy - add - disgust

    //右眉:17-21
    addx[17] = 0.0000; addy[17] = 0.0000; addz[17] = 0.0000; //-0.5 ~ addy ~ 0
    addx[18] = 0.0000; addy[18] = -changeEmo0125(z_score); addz[18] = 0.0000; //-0.75 ~ addy ~ -0.125
    addx[19] = 0.0000; addy[19] = -changeEmo025(z_score); addz[19] = 0.0000; //-0.75 ~ addy ~ -0.25
    addx[20] = 0.0000; addy[20] = -changeEmo025(z_score); addz[20] = 0.0000; //-0.75 ~ addy ~ -0.25
    addx[21] = 0.0000; addy[21] = -changeEmo05(z_score); addz[21] = 0.0000; //-0.5 ~ addy ~ -0.5
    //左眉:22-26
    addx[22] = 0.0000; addy[22] = -changeEmo05(z_score); addz[22] = 0.0000; // -0.5 ~ addy ~ -0.5
    addx[23] = 0.0000; addy[23] = -changeEmo025(z_score); addz[23] = 0.0000; // -0.75 ~ addy ~ -0.25
    addx[24] = 0.0000; addy[24] = -changeEmo025(z_score); addz[24] = 0.0000; // -0.75 ~ addy ~ -0.25
    addx[25] = 0.0000; addy[25] = -changeEmo0125(z_score); addz[25] = 0.0000; // -0.75 ~ addy ~ -0.125
    addx[26] = 0.0000; addy[26] = 0.0000; addz[26] = 0.0000; // -0.5 ~ addy ~ 0
    
    //鼻筋:27-30
    addx[27] = 0.0000; addy[27] = 0.0000; addz[27] = 0.0000;
    addx[28] = 0.0000; addy[28] = 0.0000; addz[28] = 0.0000;
    addx[29] = 0.0000; addy[29] = 0.0000; addz[29] = 0.0000;
    addx[30] = 0.0000; addy[30] = 0.0000; addz[30] = 0.0000;
    //鼻底:31-35
    addx[31] = 0.0000; addy[31] = 0.0000; addz[31] = 0.0000;
    addx[32] = 0.0000; addy[32] = 0.0000; addz[32] = 0.0000;
    addx[33] = 0.0000; addy[33] = 0.0000; addz[33] = 0.0000;
    addx[34] = 0.0000; addy[34] = 0.0000; addz[34] = 0.0000;
    addx[35] = 0.0000; addy[35] = 0.0000; addz[35] = 0.0000;
    
    //右目:36-41
    addx[36] = -changeEmo025(z_score); addy[36] = -changeEmo0125(z_score); addz[36] = 0.0000; // -0.25 ~ addx ~ -0.25, -0.125 ~ addy ~ 0.125
    addx[37] = 0.0000; addy[37] = 0.0000; addz[37] = 0.0000; //-0.450 ~ addy ~ 0.250
    addx[38] = 0.0000; addy[38] = 0.0000; addz[38] = 0.0000; //-0.450 ~ addy ~ 0.250
    addx[39] = changeEmo025(z_score); addy[39] = changeEmo0125(z_score); addz[39] = 0.0000; //0.25 ~ addx ~ 0.25, -0.1250 ~ addy ~ 0.1250
    addx[40] = 0.0000; addy[40] = 0.0000; addz[40] = 0.0000;
    addx[41] = 0.0000; addy[41] = 0.0000; addz[41] = 0.0000;
    //左目:42-47
    addx[42] = -changeEmo025(z_score); addy[42] = changeEmo0125(z_score); addz[42] = 0.0000; // -0.25 ~ addx ~ -0.25, -0.125 ~ addy ~ 0.125
    addx[43] = 0.0000; addy[43] =0.0000; addz[43] = 0.0000; //-0.450 ~ addy ~ 0.250
    addx[44] = 0.0000; addy[44] = 0.0000; addz[44] = 0.0000; //-0.450 ~ addy ~ 0.250
    addx[45] = changeEmo025(z_score); addy[45] = changeEmo0125(z_score); addz[45] = 0.0000; //0.25 ~ addx ~ 0.25, -0.1250 ~ addy ~ 0.1250
    addx[46] = 0.0000; addy[46] = 0.0000; addz[46] = 0.0000;
    addx[47] = 0.0000; addy[47] = 0.0000; addz[47] = 0.0000;
    
    //口:48-65
    //左端
    addx[48] = -changeEmo0125(z_score); addy[48] = changeEmo0375(z_score); addz[48] = 0.0000; //-1 ~ addx ~ -0.125, -1 ~ addy ~ 0.375
    
    //右端
    addx[54] = changeEmo0125(z_score); addy[54] = changeEmo0375(z_score); addz[54] = 0.0000; //-1 ~ addx ~ -0.125, -1 ~ addy ~ 0.375
    
    //上唇上側
    addx[49] = 0.0000; addy[49] = 0.0000; addz[49] = 0.0000;
    addx[50] = 0.0000; addy[50] = 0.0000; addz[50] = 0.0000;
    addx[51] = 0.0000; addy[51] = 0.0000; addz[51] = 0.0000;
    addx[52] = 0.0000; addy[52] = 0.0000; addz[52] = 0.0000;
    addx[53] = 0.0000; addy[53] = 0.0000; addz[53] = 0.0000;
    //上唇下側
    addx[60] = 0.0000; addy[60] = 0.0000; addz[60] = 0.0000;
    addx[61] = 0.0000; addy[61] = 0.0000; addz[61] = 0.0000;
    addx[62] = 0.0000; addy[62] = 0.0000; addz[62] = 0.0000;
    
    //下唇上側
    addx[63] = 0.0000; addy[63] = 0.0000; addz[63] = 0.0000;
    addx[64] = 0.0000; addy[64] = 0.0000; addz[64] = 0.0000;
    addx[65] = 0.0000; addy[65] = 0.0000; addz[65] = 0.0000;
    //下唇下側
    addx[55] = 0.0000; addy[55] = 0; addz[55] = 0.0000;
    addx[56] = 0.0000; addy[56] = 0; addz[56] = 0.0000;
    addx[57] = 0.0000; addy[57] = 0; addz[57] = 0.0000;
    addx[58] = 0.0000; addy[58] = 0; addz[58] = 0.0000;
    addx[59] = 0.0000; addy[59] = 0; addz[59] = 0.0000;
    
    
}

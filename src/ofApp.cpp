#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetVerticalSync(true);
	ofSetLogLevel(OF_LOG_NOTICE);
	
	densityWidth = 1280;
	densityHeight = 720;
	simulationWidth = densityWidth / 2;
	simulationHeight = densityHeight / 2;
	windowWidth = ofGetWindowWidth();
	windowHeight = ofGetWindowHeight();
	
	opticalFlow.setup(simulationWidth, simulationHeight);
//	velocityBridgeFlow.setup(simulationWidth, simulationHeight);
//	densityBridgeFlow.setup(simulationWidth, simulationHeight, densityWidth, densityHeight);
//	temperatureBridgeFlow.setup(simulationWidth, simulationHeight);
	combinedBridgeFlow.setup(simulationWidth, simulationHeight, densityWidth, densityHeight);
	fluidFlow.setup(simulationWidth, simulationHeight, densityWidth, densityHeight);
	
	flows.push_back(&opticalFlow);
//	flows.push_back(&velocityBridgeFlow);
//	flows.push_back(&densityBridgeFlow);
//	flows.push_back(&temperatureBridgeFlow);
	flows.push_back(&combinedBridgeFlow);
	flows.push_back(&fluidFlow);
	
	//��Q���Ƃ��Đݒ�
	flowToolsLogo.load("flowtools.png");
	fluidFlow.addObstacle(flowToolsLogo.getTexture());
	//flowToolsLogo.mirror(false, true);
	
	simpleCam.setup(densityWidth, densityHeight, true);
	cameraFbo.allocate(densityWidth, densityHeight);
	ftUtil::zero(cameraFbo);
	
	lastTime = ofGetElapsedTimef();
	
	setupGui();

	//�T�E���h(�K�v�Ȃ�)
	//soundPlayer.load("");
	//soundPlayer.setLoop(true);
	//soundPlayer.play();
	
	//�v���J�n���Ԏ擾
	//start_time_ = clock();
	//�}�E�X�J�[�\�����B��
	ofHideCursor();
}

//--------------------------------------------------------------
void ofApp::setupGui() {
	//setting.xml�̍쐬(���ɍ���Ă�����ǂݍ���)
	gui.setup("settings");//���O
	gui.setDefaultBackgroundColor(ofColor(0, 0, 0, 127));//�w�i
	gui.setDefaultFillColor(ofColor(160, 160, 160, 160));//�h��Ԃ��̐F
	gui.add(guiFPS.set("average FPS", 0, 0, 60));//����FPS
	gui.add(guiMinFPS.set("minimum FPS", 0, 0, 60));//�ŏ�FPS
	gui.add(toggleFullScreen.set("fullscreen (F)", true));//�t���X�N���[���\��
	toggleFullScreen.addListener(this, &ofApp::toggleFullScreenListener);
	gui.add(toggleGuiDraw.set("show gui (G)", true));//GUI(�f�o�b�O�p)
	gui.add(toggleCameraDraw.set("draw camera (C)", false));//�J�����\��(�f�o�b�O�p)
	gui.add(toggleReset.set("reset (R)", false));//���Z�b�g(�f�o�b�O�p)
	toggleReset.addListener(this, &ofApp::toggleResetListener);
	gui.add(outputWidth.set("output width", 1280, 256, 1920));//�ȉ����̓����̃f�o�b�O�p
	gui.add(outputHeight.set("output height", 720, 144, 1080));
	gui.add(simulationScale.set("simulation scale", 2, 1, 4));
	gui.add(simulationFPS.set("simulation fps", 60, 1, 60));
	outputWidth.addListener(this, &ofApp::simulationResolutionListener);
	outputHeight.addListener(this, &ofApp::simulationResolutionListener);
	simulationScale.addListener(this, &ofApp::simulationResolutionListener);
	
	visualizationParameters.setName("visualization");
	visualizationParameters.add(visualizationMode.set("mode", FLUID_DEN, INPUT_FOR_DEN, FLUID_DEN));
	visualizationParameters.add(visualizationName.set("name", "fluidFlow Density"));
	visualizationParameters.add(visualizationScale.set("scale", 1, 0.1, 10.0));
	visualizationParameters.add(toggleVisualizationScalar.set("show scalar", false));
	visualizationMode.addListener(this, &ofApp::visualizationModeListener);
	toggleVisualizationScalar.addListener(this, &ofApp::toggleVisualizationScalarListener);
	visualizationScale.addListener(this, &ofApp::visualizationScaleListener);
	
	bool s = true;
	switchGuiColor(s = !s);
	//�e�p�����[�^�ǉ�
	gui.add(visualizationParameters);
	for (auto flow : flows) {
		switchGuiColor(s = !s);
		gui.add(flow->getParameters());
	}
	//�Z�[�u(2��ڈȍ~�̓��[�h)
	if (!ofFile("settings.xml")) { gui.saveToFile("settings.xml"); }
	gui.loadFromFile("settings.xml");
	
	gui.minimizeAll();
	toggleGuiDraw = false;//true
}

//--------------------------------------------------------------
void ofApp::switchGuiColor(bool _switch) {
	//GUI
	ofColor guiHeaderColor[2]; 
	guiHeaderColor[0].set(255, 0, 0, 0);//160, 160, 80, 200
	guiHeaderColor[1].set(255, 0, 0, 0);//80, 160, 160, 200
	ofColor guiFillColor[2];
	guiFillColor[0].set(255, 0, 0, 0);//160, 160, 80, 200
	guiFillColor[1].set(255, 0, 0, 0);//80, 160, 160, 200
	
	gui.setDefaultHeaderBackgroundColor(guiHeaderColor[_switch]);
	gui.setDefaultFillColor(guiFillColor[_switch]);
}

//--------------------------------------------------------------
void ofApp::update(){
	//�o�ߎ��Ԃ̎擾
	//clock_t end = clock();

	//���������b���ɕϊ�
	//double sec = (double)(end - start_time_) / CLOCKS_PER_SEC;

	//�K�莞�Ԃ��o�߂����玩�g���폜
	/*if (sec >= total_sec_) {
		ofExit();
		return;
	}*/

	//�t���[�����[�g��ݒ�
	ofSetFrameRate(simulationFPS);
	
	float dt = 1.0 / max(ofGetFrameRate(), 1.f); 
	
	//�J�����X�V
	simpleCam.update();
	if (simpleCam.isFrameNew()) {
		cameraFbo.begin();
		ofEnableBlendMode(OF_BLENDMODE_DISABLED);//OF_BLENDMODE_DISABLED
		simpleCam.draw(cameraFbo.getWidth(), 0, -cameraFbo.getWidth(), cameraFbo.getHeight());//�e�X�g���p 
		//simpleCam.draw(0, cameraFbo.getHeight(), cameraFbo.getWidth(), -cameraFbo.getHeight());//�{�Ԋ��p 
		/*
		simpleCam.draw(cameraFbo.getWidth(), cameraFbo.getHeight(), -cameraFbo.getWidth(), -cameraFbo.getHeight()); 
		simpleCam.draw(0, cameraFbo.getHeight(), cameraFbo.getWidth(), -cameraFbo.getHeight()); 
		simpleCam.draw(cameraFbo.getWidth(), 0, -cameraFbo.getWidth(), cameraFbo.getHeight()); 
		simpleCam.draw(0, 0, cameraFbo.getWidth(), cameraFbo.getHeight()); 
		*/
		cameraFbo.end();
		opticalFlow.setInput(cameraFbo.getTexture());
	}
	//�X�V
	opticalFlow.update(); 
	//opticalflow�X�V
	combinedBridgeFlow.setVelocity(opticalFlow.getVelocity());
	combinedBridgeFlow.setDensity(cameraFbo.getTexture());
	combinedBridgeFlow.update(dt); 

//	velocityBridgeFlow.setVelocity(opticalFlow.getVelocity());
//	velocityBridgeFlow.update(dt);
//	densityBridgeFlow.setDensity(cameraFbo.getTexture());
//	densityBridgeFlow.setVelocity(opticalFlow.getVelocity());
//	densityBridgeFlow.update(dt);
//	temperatureBridgeFlow.setDensity(cameraFbo.getTexture());
//	temperatureBridgeFlow.setVelocity(opticalFlow.getVelocity());
//	temperatureBridgeFlow.update(dt);
	
	//opticalflow����v�Z�������̂�����
	fluidFlow.addVelocity(combinedBridgeFlow.getVelocity());
	fluidFlow.addDensity(combinedBridgeFlow.getDensity());
	fluidFlow.addTemperature(combinedBridgeFlow.getTemperature());
	fluidFlow.update(dt);
}

//--------------------------------------------------------------
void ofApp::draw(){
	//������
	ofClear(0,0);
	//�J����
	ofPushStyle();
	if (toggleCameraDraw.get()) {
		ofEnableBlendMode(OF_BLENDMODE_DISABLED);
		//ofEnableAlphaBlending();
		cameraFbo.draw(0, 0, windowWidth, windowHeight);
	}
	
	//��
	ofEnableBlendMode(OF_BLENDMODE_ADD);//OF_BLENDMODE_ALPHA / OF_BLENDMODE_ADD / OF_BLENDMODE_SCREEN 
	//ofEnableAlphaBlending();
	switch(visualizationMode.get()) {
		case INPUT_FOR_DEN:	combinedBridgeFlow.drawInput(0, 0, windowWidth, windowHeight); break;
		case INPUT_FOR_VEL:	opticalFlow.drawInput(0, 0, windowWidth, windowHeight); break;
		case FLOW_VEL:		opticalFlow.draw(0, 0, windowWidth, windowHeight); break;
		case BRIDGE_VEL:	combinedBridgeFlow.drawVelocity(0, 0, windowWidth, windowHeight); break;
		case BRIDGE_DEN:	combinedBridgeFlow.drawDensity(0, 0, windowWidth, windowHeight); break;
		case BRIDGE_TMP:	combinedBridgeFlow.drawTemperature(0, 0, windowWidth, windowHeight); break;
		case BRIDGE_PRS:	break;
		case OBSTACLE:		fluidFlow.drawObstacle(0, 0, windowWidth, windowHeight); break;
		case OBST_OFFSET:	fluidFlow.drawObstacleOffset(0, 0, windowWidth, windowHeight); break;
		case FLUID_BUOY:	fluidFlow.drawBuoyancy(0, 0, windowWidth, windowHeight); break;
		case FLUID_VORT:	fluidFlow.drawVorticity(0, 0, windowWidth, windowHeight); break;
		case FLUID_DIVE:	fluidFlow.drawDivergence(0, 0, windowWidth, windowHeight); break;
		case FLUID_TMP:		fluidFlow.drawTemperature(0, 0, windowWidth, windowHeight); break;
		case FLUID_PRS:		fluidFlow.drawPressure(0, 0, windowWidth, windowHeight); break;
		case FLUID_VEL:		fluidFlow.drawVelocity(0, 0, windowWidth, windowHeight); break;
		case FLUID_DEN:		fluidFlow.draw(0, 0, windowWidth, windowHeight); break;
		default: break;
	}
	//���S 
	ofEnableBlendMode(OF_BLENDMODE_SUBTRACT);
	flowToolsLogo.draw(0, 0, windowWidth, windowHeight);
	
	//GUI
	if (toggleGuiDraw) { 
		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		drawGui();
	}
	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::drawGui() {
	guiFPS = (int)(ofGetFrameRate() + 0.5);
	
	//�ŏ�fps�̌v�Z
	float deltaTime = ofGetElapsedTimef() - lastTime;
	lastTime = ofGetElapsedTimef();
	deltaTimeDeque.push_back(deltaTime);
	
	while (deltaTimeDeque.size() > guiFPS.get())
		deltaTimeDeque.pop_front();
	
	float longestTime = 0;
	for (int i=0; i<(int)deltaTimeDeque.size(); i++){
		if (deltaTimeDeque[i] > longestTime) longestTime = deltaTimeDeque[i];
	}
	
	guiMinFPS.set(1.0 / longestTime);
	
	ofPushStyle();
	ofEnableBlendMode(OF_BLENDMODE_ALPHA); 
	gui.draw();
	ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	//�f�o�b�O�p
	switch (key) {
		default: break;
		case '1': visualizationMode.set(INPUT_FOR_DEN); break;//�`�惂�[�h�ύX(1�`0)
		case '2': visualizationMode.set(INPUT_FOR_VEL); break;
		case '3': visualizationMode.set(FLOW_VEL); break;
		case '4': visualizationMode.set(BRIDGE_VEL); break;
		case '5': visualizationMode.set(BRIDGE_DEN); break;
		case '6': visualizationMode.set(FLUID_VORT); break;
		case '7': visualizationMode.set(FLUID_TMP); break;
		case '8': visualizationMode.set(FLUID_PRS); break;
		case '9': visualizationMode.set(FLUID_VEL); break;
		case '0': visualizationMode.set(FLUID_DEN); break;
		case 'G':toggleGuiDraw = !toggleGuiDraw; break;//GUI�\��/��\��
		case 'F': ofExit(); break;//�폜
		case 'C': toggleCameraDraw.set(!toggleCameraDraw.get()); break;//�J�����\��/��\��
		case 'R': toggleReset.set(!toggleReset.get()); break;//���Z�b�g
	}
}

//--------------------------------------------------------------
void ofApp::toggleResetListener(bool& _value) {
	if (_value) {
		for (auto flow : flows) { flow->reset(); }
		fluidFlow.addObstacle(flowToolsLogo.getTexture());
	}
	_value = false;
}

void ofApp::simulationResolutionListener(int &_value){
	//�Čv�Z�p
	densityWidth = outputWidth;
	densityHeight = outputHeight;
	simulationWidth = densityWidth / simulationScale;
	simulationHeight = densityHeight / simulationScale;
	
	opticalFlow.resize(simulationWidth, simulationHeight);
	combinedBridgeFlow.resize(simulationWidth, simulationHeight, densityWidth, densityHeight);
	
	fluidFlow.resize(simulationWidth, simulationHeight, densityWidth, densityHeight);
	fluidFlow.addObstacle(flowToolsLogo.getTexture());
}
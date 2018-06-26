#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include<iostream>
using namespace cv;
using namespace std;

int frameSlider = 0;
int currentFrame = 0;
int max_frames = 0;

int fpsSlider = 30;
int currentFps = 30;
int max_fps = 60;

bool prevScene = false, nextScene = false;

void on_trackbar(int, void*)
{
	prevScene = false;
	nextScene = false;
	currentFrame = frameSlider;
}
void on_trackbar1(int, void*)
{
	currentFps = fpsSlider;
}

int main() {
	//Utworzenie macierzy przechowujacej obraz ( o dowolnych, skonczonych wymiarach)
	Mat source, editor, logo, image, mask;
	int fps, frameCount, pressedButton, prevSceneFrame, nextSceneFrame, recordFrame, x = 0, y = 0, option, startNumber;
	bool recording = false, endKeyPressed = false, isVideo = false, isLogo = false;
	string filename, outputSequence, fileExtension, message;
	VideoCapture cap, capEditor;
	VideoWriter oVideoWriter;
	Size frameSize(static_cast<int>(640), static_cast<int>(480));
	cout << "\n1. Add logo to image";
	cout << "\n2. Edit Video or camera to video";
	cout << "\n3. Edit Video to images";
	cout << "\n4. Create video from images\n";
	cin >> option;
	if (option == 1 || option == 3) message = "Write filename and extension: ";
	else if (option == 4) message = "Write filename: ";
	else if (option == 2) message = "Set filename(to open camera, write camera): ";
	cout << message;
	cin >> filename;
	if (option == 4) {
		cout << "\nWrite file extension: ";
		cin >> fileExtension;
		cout << "\nWrite start number: ";
		cin >> startNumber;
	}
	if (option == 2 || option == 3) {
		//Inicjalizacja przechwytywania
		if (filename == "camera") {
			cap.open(0); // 0 wbudowana kamera laptopa, 1 kamera zewnetrzna
			if (!cap.isOpened()) {
				cout << "Cant open stream";
				return -1;
			}
		}
		else {
			isVideo = true;
			cap = VideoCapture(filename);
			if (!cap.isOpened()) {
				cout << "Cant open stream";
				return -1;
			}
			else {
				capEditor = VideoCapture(filename);
				frameCount = cap.get(CAP_PROP_FRAME_COUNT);
				max_frames = frameCount - 1;
				capEditor >> editor;
				editor.copyTo(source);
			}
		}
	}
	else if (option == 1) {
		source = imread(filename, -1);
		if (source.empty()) {
			return -1;
		}

	}
	else if (option == 4) {
		string output;
		cout << "\nSet output filename: ";
		cin >> output;
		cout << "\nSet fps: ";
		cin >> currentFps;
		oVideoWriter = VideoWriter(output + ".avi", CV_FOURCC('D', 'I', 'V', 'X'), currentFps, frameSize, true);
		while (1) {
			source = imread(filename + to_string(startNumber) + "." + fileExtension, -1);
			if (source.empty()) {
				return -1;
			}
			oVideoWriter.write(source);
			startNumber++;
		}
		cap.release();
		oVideoWriter.release();
		cvDestroyAllWindows();
		return 0;
	}
	//Ustalenie rozmiaru okna o wymiarach (640, 480)
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 640);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
	//Utworzenie okna w celu pokazania obrazu z kamery
	namedWindow("window", CV_WINDOW_AUTOSIZE);
	if (option != 1 && filename != "camera")namedWindow("editor", CV_WINDOW_AUTOSIZE);
	if (option != 1) {
		namedWindow("trackbar", CV_WINDOW_AUTOSIZE);

		if (filename != "camera") createTrackbar("frame", "trackbar", &frameSlider, max_frames, on_trackbar);
		if (option != 3)createTrackbar("fps", "trackbar", &fpsSlider, max_fps, on_trackbar1);
	}

	while (1) {
		//kopiowanie obazu z kamery
		if (!isVideo && (option == 2 || option == 3)) {
			cap >> source;
		}
		else if (option == 2 || option == 3) {
			capEditor.set(CV_CAP_PROP_POS_FRAMES, currentFrame);
			capEditor >> editor;
		}
		if (isLogo) {
			if (isVideo) logo.copyTo(editor(cv::Rect(x, y, logo.cols, logo.rows)), mask);
			if (option == 1) {
				source = imread(filename, -1);
				if (source.empty()) {
					return -1;
				}
			}
			logo.copyTo(source(cv::Rect(x, y, logo.cols, logo.rows)), mask);
		}

		if (recording) {
			if (!isVideo && option == 2) oVideoWriter.write(source);
			else if (option == 3) {
				vector<int> compression_params;
				compression_params.push_back(80);
				compression_params.push_back(9);
				cap.set(CV_CAP_PROP_POS_FRAMES, recordFrame);
				cap >> source;
				if (isLogo) logo.copyTo(source(cv::Rect(x, y, logo.cols, logo.rows)), mask);
				imwrite(outputSequence + to_string(recordFrame) + ".jpg", source, compression_params);
				setTrackbarPos("frame", "trackbar", recordFrame);
				if (recordFrame < max_frames) recordFrame++;
				else {
					recording = false;
					cout << "\nEnded recording - end of file";
				}
			}
			else if (option == 2) {
				cap.set(CV_CAP_PROP_POS_FRAMES, recordFrame);
				cap >> source;
				if (isLogo) logo.copyTo(source(cv::Rect(x, y, logo.cols, logo.rows)), mask);
				setTrackbarPos("frame", "trackbar", recordFrame);
				oVideoWriter.write(source);
				if (recordFrame < max_frames) recordFrame++;
				else {
					recording = false;
					oVideoWriter.release();
					cout << "\nEnded recording - end of file";
				}
			}
		}
		if (prevScene) {
			nextScene = false;
			cap.set(CV_CAP_PROP_POS_FRAMES, prevSceneFrame);
			if (prevSceneFrame > 0) prevSceneFrame--;
			else {
				setTrackbarPos("frame", "trackbar", currentFrame);
				prevScene = false;
			}
			cap >> source;
		}
		else if (nextScene) {
			prevScene = false;
			cap.set(CV_CAP_PROP_POS_FRAMES, nextSceneFrame);
			if (nextSceneFrame < max_frames) nextSceneFrame++;
			else {
				setTrackbarPos("frame", "trackbar", currentFrame);
				nextScene = false;
			}
			cap >> source;
		}
		else if (isVideo && !recording && !nextScene && !prevScene && (option == 2 || option == 3)) {
			setTrackbarPos("frame", "trackbar", currentFrame);
			editor.copyTo(source);
		}
		//wyswietlanie skopiowanego obrazu z kamery w oknie
		if (isVideo)imshow("editor", editor);
		imshow("window", source);

		//opoznienie o wartosci 15 milisekund (sprawdzic co bedzie jesli nie ma opoznienia)
		pressedButton = waitKeyEx(15);


		/*MENU*/
		//menu dla video

		//start -s
		if (pressedButton == 115 && !recording && (option == 2 || option == 3)) {
			if (!isVideo && option == 2) {
				string output;
				cout << "\nSet output filename: ";
				cin >> output;
				oVideoWriter = VideoWriter(output + ".avi", CV_FOURCC('D', 'I', 'V', 'X'), currentFps, frameSize, true);
			}
			else if (isVideo && option == 3) {
				cout << "\nSet output filename: ";
				cin >> outputSequence;
				recordFrame = currentFrame;
			}
			else if (isVideo && option == 2) {
				recordFrame = currentFrame;
				string output;
				cout << "\nSet output filename: ";
				cin >> output;
				oVideoWriter = VideoWriter(output + ".avi", CV_FOURCC('D', 'I', 'V', 'X'), currentFps, frameSize, true);
			}
			recording = true;
			cout << "\nStarted recording...";
		}
		else if (pressedButton == 115 && option == 1) {
			vector<int> compression_params;
			compression_params.push_back(100);
			compression_params.push_back(9);
			string output;
			cout << "\nSet output filename: ";
			cin >> output;
			imwrite(output + ".jpg", source, compression_params);
			cout << "Succesfully created an image!";
		}
		//end - e
		else if (pressedButton == 101 && recording && (option == 2 || option == 3)) {
			oVideoWriter.release();
			recording = false;
			if (isVideo) setTrackbarPos("frame", "trackbar", --recordFrame);
			cout << "\nEnded recording";
			cout << "\nSuccesfully created a video!";
		}
		//Logo - l
		else if (pressedButton == 108) {
			if (!isLogo) {
				string input;
				cout << "\nWrite logo filename and file extension: ";
				cin >> input;
				logo = imread(input, -1);
				if (logo.empty()) {
					return -1;
				}
				vector<Mat> rgbLayer;
				if (logo.channels() == 4)
				{
					split(logo, rgbLayer);         // seperate channels
					Mat cs[3] = { rgbLayer[0],rgbLayer[1],rgbLayer[2] };
					merge(cs, 3, logo);  // glue together again
					mask = rgbLayer[3];       // png's alpha channel used as mask
				}
				isLogo = true;
			}
			else isLogo = false;
		}
		else if (pressedButton == 27) break;

		if (isVideo) {
			//Prev frame - o
			if (pressedButton == 111) {
				currentFrame--;
				if (currentFrame < 0) currentFrame = 0;
			}
			//Prev keyframe - ctrl - o
			else if (pressedButton == 15) {
				currentFrame -= 15;
				if (currentFrame < 0) currentFrame = 0;
			}
			//back 50 frames - shift - o
			else if (pressedButton == 79) {
				currentFrame -= 50;
				if (currentFrame < 0) currentFrame = 0;
			}
			//Next frame - p
			else if (pressedButton == 112) {
				currentFrame++;
				if (currentFrame >= frameCount) currentFrame = max_frames;
			}
			//Next keyframe - ctrl - p
			else if (pressedButton == 16) {
				currentFrame += 15;
				if (currentFrame >= frameCount) currentFrame = max_frames;
			}
			//Forward 50 frames - shift - p
			else if (pressedButton == 80) {
				currentFrame += 50;
				if (currentFrame >= frameCount) currentFrame = max_frames;
			}
			//Beggining - b
			else if (pressedButton == 98) {
				currentFrame = 0;
			}
			//Finish - f
			else if (pressedButton == 102) {
				currentFrame = max_frames;
			}
			//Prev scene - left
			else if (pressedButton == 2424832) {
				if (!isLogo) {
					prevSceneFrame = currentFrame;
					currentFrame = 0;
					prevScene = true;
				}
			}
			//Next scene - right
			else if (pressedButton == 2555904) {
				if (!isLogo) {
					nextSceneFrame = currentFrame;
					currentFrame = max_frames;
					nextScene = true;
				}
			}
		}
		if (isLogo) {
			//Logo left - left
			if (pressedButton == 2424832) {
				if (x > 0) x--;
			}
			//Logo right - right
			else if (pressedButton == 2555904) {
				if (x < source.cols - logo.cols)x++;
			}
			//Logo up - up
			else if (pressedButton == 2490368) {
				if (y > 0) y--;
			}
			//Logo down - down
			else if (pressedButton == 2621440) {
				if (y < source.rows - logo.rows) y++;
			}
		}
	}
	cap.release();
	oVideoWriter.release();
	cvDestroyAllWindows();
	return 0;
}

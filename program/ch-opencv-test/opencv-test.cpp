#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>

using namespace std;

void read_image(string file) {
	cout << "\nReading image " << file << " ..." << endl;
	cv::Mat img = cv::imread(file, -1);
	if (img.empty())
		throw runtime_error("Unable to decode image");
	cout << "OK\n";
}

int main() {
	try {
		read_image("../test.png");
		read_image("../test.jpg");
		read_image("../test.tif");
	}
  catch (runtime_error &err)
  {
    cerr << "\nERROR: " << err.what() << endl;
    return -1;
  }  
  return 0;
}

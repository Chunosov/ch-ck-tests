#include "detector.h"

#include <chrono>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

using namespace std;
using namespace std::chrono;
namespace fs = boost::filesystem;

int getenv_i(const char* name, int def) {
  return getenv(name) ? atoi(getenv(name)) : def;
}

string getenv_s(const char* name) {
  const char* val = getenv(name);
  if (!val || strlen(val) == 0) {
    cerr << "ERROR: env var is not set: " << name << endl;
    exit(1);
  }
  return string(val);
}

const int BATCH_COUNT = getenv_i("CK_BATCH_COUNT", 1);
const int BATCH_SIZE = getenv_i("CK_BATCH_SIZE", 1);
const int IMAGES_COUNT = BATCH_COUNT * BATCH_SIZE;
const int SKIP_IMAGES = getenv_i("CK_SKIP_IMAGES", 0);
const string IMAGES_DIR = getenv_s("CK_ENV_DATASET_IMAGE_DIR");
const string LABEL_MAP = getenv_s("CK_ENV_MODEL_CAFFE_LABELMAP");
const string WEIGHTS_FILE = getenv_s("CK_ENV_MODEL_CAFFE_WEIGHTS");
const string TMP_MODEL_FILE = "tmp.prototxt";
const string MODEL_FILE = (fs::path(getenv_s("CK_ENV_MODEL_CAFFE"))/"deploy.prototxt").native();
const string MEAN_FILE = "";
const string MEAN_VALUE = "104,117,123"; // It came from original example
const float CONF_THRESHOLD = 0.01; // It came from original example

vector<string> get_images() {
  const string filter1(".JPG");
  const string filter2(".JPEG");

  vector<string> all_images;
  fs::directory_iterator end_iter;
  for (fs::directory_iterator dir_iter(IMAGES_DIR) ; dir_iter != end_iter ; ++dir_iter){
    if (!fs::is_regular_file(dir_iter->status())) continue;

    string file = dir_iter->path().string();

    string tmp = boost::to_upper_copy(file);
    if (tmp.compare(file.size()-filter1.size(), filter1.size(), filter1) != 0 &&
        tmp.compare(file.size()-filter2.size(), filter2.size(), filter2) != 0) continue;

    all_images.push_back(file);
  }

  sort(all_images.begin(), all_images.end());

  int last_index = SKIP_IMAGES + IMAGES_COUNT;
  if (last_index >= all_images.size())
    last_index = all_images.size()-1;

  return vector<string>(&all_images[SKIP_IMAGES], &all_images[last_index]);
}

template <typename T>
void str_replace(string& str, const string& from, const T& to) {
  size_t pos = str.find(from);
  if (pos != string::npos) {
    stringstream tmp; tmp << to;
    str.replace(pos, from.size(), tmp.str());
  }
}

int main(int argc, char** argv) {
  // Otherwise caffe will flood stderr with lot of odd messages
  ::google::InitGoogleLogging(argv[0]);

  cout << "Model file: " << MODEL_FILE << endl;
  cout << "Weights file: " << WEIGHTS_FILE << endl;
  cout << "Label map file: " << LABEL_MAP << endl;
  cout << "Images dir: " << IMAGES_DIR << endl;
  cout << "Batch count: " << BATCH_COUNT << endl;
  cout << "Batch size: " << BATCH_SIZE << endl;

  // Prepare prototxt
  cout << endl << "Preparing prototxt..." << endl;
  stringstream s; s << ifstream(MODEL_FILE).rdbuf();
  string prototxt = s.str();
  str_replace(prototxt, "$#batch_size#$", BATCH_SIZE);
  str_replace(prototxt, "$#path_to_labelmap#$", LABEL_MAP);
  ofstream(TMP_MODEL_FILE, ofstream::trunc) << prototxt;

  // Load processing image filenames
  cout << endl << "Loading image list..." << endl;
  vector<string> images = get_images();

  // Build net
  cout << endl << "Initializing detector..." << endl;
  time_point<high_resolution_clock> start_time = high_resolution_clock::now();
  Detector detector(TMP_MODEL_FILE, WEIGHTS_FILE, MEAN_FILE, MEAN_VALUE);
  duration<double> elapsed = high_resolution_clock::now() - start_time;  
  cout << "Detector initialised in " << elapsed.count() << "s" << endl;

  // Run batched mode
  cout << endl << "Detect..." << endl;
  double load_total_time = 0;
  double det_total_time = 0;
  int image_index = 0;
  int images_processed = 0;
  for (int batch_index = 0; batch_index < BATCH_COUNT; batch_index++) {
    cout << "Batch " << batch_index << endl;

    // Detect batch
    for (int i = 0; i < BATCH_SIZE; i++) {
      cout << endl << fs::path(images[image_index]).filename().native() << endl;

      // Load image
      start_time = high_resolution_clock::now();
      cv::Mat img = cv::imread(images[image_index], -1);
      CHECK(!img.empty()) << "Unable to decode image " << images[image_index];
      elapsed = high_resolution_clock::now() - start_time;  
      load_total_time += elapsed.count();

      // Detect
      start_time = high_resolution_clock::now();
      vector<Detection> dets = detector.Detect(img);
      elapsed = high_resolution_clock::now() - start_time;

      // Print detections
      for (auto det: dets)
        if (det.score >= CONF_THRESHOLD)
          cout << det.str() << endl;

      // Exclude first batch from averaging
      if (batch_index > 0 || BATCH_COUNT == 1) {
        det_total_time += elapsed.count();
        images_processed += BATCH_SIZE;
      }

      image_index++;
    }
  }

  double det_avg_time = det_total_time / double(images_processed);

  cout << endl;
  cout << "Images processed: " << images_processed << endl;
  cout << "All images loaded in " << load_total_time << "s" << endl;
  cout << "All images detected in " << det_total_time << "s" << endl;
  cout << "Average detection time: " << det_avg_time << "s";
  if (BATCH_COUNT > 1) cout << " (first batch excluded)";
  cout << endl;

  return 0;
}

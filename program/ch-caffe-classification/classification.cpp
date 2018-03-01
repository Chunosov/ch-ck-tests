#include "classifier.h"

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
const int BATCH_SIZE = getenv_i("CK_CAFFE_BATCH_SIZE", 1);
const int IMAGES_COUNT = BATCH_COUNT * BATCH_SIZE;
const int SKIP_IMAGES = getenv_i("CK_SKIP_IMAGES", 0);
const string IMAGES_DIR = getenv_s("CK_ENV_DATASET_IMAGENET_VAL");

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

int main(int argc, char** argv) {
  // Otherwise caffe will flood stderr with lot of odd messages
  ::google::InitGoogleLogging(argv[0]);

  fs::path caffemodel_path(getenv_s("CK_ENV_MODEL_CAFFE"));
  string model_file = (caffemodel_path / "deploy.prototxt").native();
  string weights_file = getenv_s("CK_ENV_MODEL_CAFFE_WEIGHTS");
  string tmp_model_file = "tmp.prototxt";

  fs::path aux_path(getenv_s("CK_ENV_DATASET_IMAGENET_AUX"));
  string mean_file = (aux_path / "imagenet_mean.binaryproto").native();
  string labels_file = (aux_path / "synset_words.txt").native();

  cout << "Model file: " << model_file << endl;
  cout << "Weights file: " << weights_file << endl;
  cout << "Mean file: " << mean_file << endl;
  cout << "Labels file: " << labels_file << endl;
  cout << "Images dir: " << IMAGES_DIR << endl;
  cout << "Batch count: " << BATCH_COUNT << endl;
  cout << "Batch size: " << BATCH_SIZE << endl;

  // Prepare prototxt
  cout << endl << "Preparing prototxt..." << endl;
  stringstream s; s << ifstream(model_file).rdbuf();
  string prototxt = s.str();
  size_t pos = prototxt.find("$#batch_size#$");
  if (pos != string::npos) {
    stringstream s; s << BATCH_SIZE;
    prototxt.replace(pos, strlen("$#batch_size#$"), s.str());
    ofstream(tmp_model_file, ofstream::trunc) << prototxt;
  }
  else tmp_model_file = model_file;

  // Load processing image filenames
  cout << endl << "Loading image list..." << endl;
  vector<string> images = get_images();

  // Build net
  cout << endl << "Initializing classifier..." << endl;
  time_point<high_resolution_clock> start_time = high_resolution_clock::now();
  Classifier classifier(tmp_model_file, weights_file, mean_file, labels_file);
  duration<double> elapsed = high_resolution_clock::now() - start_time;  
  cout << "Classifier initialised in " << elapsed.count() << "s" << endl;

  // Run batched mode
  cout << endl << "Classify..." << endl;
  double load_total_time = 0;
  double class_total_time = 0;
  int image_index = 0;
  int images_processed = 0;
  for (int batch_index = 0; batch_index < BATCH_COUNT; batch_index++) {
    cout << "Batch " << batch_index << endl;

    // Classify batch
    for (int i = 0; i < BATCH_SIZE; i++) {
      // Load image
      start_time = high_resolution_clock::now();
      cv::Mat img = cv::imread(images[image_index], -1);
      cv::Mat img_prepared = classifier.PrepareImage(img);
      elapsed = high_resolution_clock::now() - start_time;  
      load_total_time += elapsed.count();
      CHECK(!img.empty()) << "Unable to decode image " << images[image_index];

      // Classify
      start_time = high_resolution_clock::now();
      vector<float> probs = classifier.Predict(img_prepared);
      elapsed = high_resolution_clock::now() - start_time;  

      // Print the top N predictions.
      vector<Prediction> predictions = classifier.ProcessPredictions(probs);
      for (size_t i = 0; i < predictions.size(); ++i) {
        Prediction p = predictions[i];
        cout << fixed << setprecision(4) << p.second << " - \"" << p.first << "\"" << endl;
      }

      // Exclude first batch from averaging
      if (batch_index > 0 || BATCH_COUNT == 1) {
        class_total_time += elapsed.count();
        images_processed += BATCH_SIZE;
      }

      image_index++;
    }
  }

  double class_avg_time = class_total_time / double(images_processed);

  cout << endl;
  cout << "Images processed: " << images_processed << endl;
  cout << "All images loaded in " << load_total_time << "s" << endl;
  cout << "All images classified in " << class_total_time << "s" << endl;
  cout << "Average classification time: " << class_avg_time << "s";
  if (BATCH_COUNT > 1) cout << " (first batch excluded)";
  cout << endl;

  return 0;
}

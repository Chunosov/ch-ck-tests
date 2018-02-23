//#define DEBUG_PARSE

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#ifdef DEBUG_PARSE
#include <bitset>
#endif

#include "lmdb.h" 

using namespace std;

struct ProtoValue {
  int tag;
  int value;
  int type;
};

struct CaffeDatum {
  int channels = 0;
  int height = 0;
  int width = 0;
  const char* data = nullptr;
  int label = 0;
  bool encoded = false;
  int image_bytes = 0;

  enum {
    TAG_CHANNELS = 1,
    TAG_HEIGHT = 2,
    TAG_WIDTH = 3,
    TAG_DATA = 4,
    TAG_LABEL = 5,
    TAG_ENCODED = 7
  };

  // Read about binary protobuf format here:
  // https://developers.google.com/protocol-buffers/docs/encoding
  ProtoValue parse_int(const char* buf, int& index) const {
    char varint_key = buf[index++];
    int type = int(varint_key & 0x7);
    int tag = int((varint_key & ~0x7) >> 3);
#ifdef DEBUG_PARSE
    cout << "index=" << index-1 << ", tag=" << tag << ", type=" << type << endl;
#endif
    if (!(tag == TAG_CHANNELS || tag == TAG_HEIGHT ||
      tag == TAG_WIDTH || tag == TAG_DATA ||
      tag == TAG_LABEL || tag == TAG_ENCODED)) {
      ostringstream s; s << "Unsupported value tag: " << tag;
      throw runtime_error(s.str());
    }
    const char MSB = 0x8<<4;
    char byte = buf[index++];
    bool has_next = byte & MSB;
    char seven_bits = byte & ~MSB;
    int value = seven_bits;
    int shift = 7;
#ifdef DEBUG_PARSE
    cout << "  index=" << index-1
         << ", byte=" << int(byte) << " (" << bitset<8>(byte) << ")"
         << ", seven_bits=" << int(seven_bits)
         << ", value=" << value 
         << ", has_next=" << has_next 
         << ", next_shift=" << shift << endl;
#endif
    while (has_next) {
      byte = buf[index++];
      has_next = byte & MSB;
      seven_bits = byte & ~MSB;
      value |= int(seven_bits) << shift;
      shift += 7;
#ifdef DEBUG_PARSE
      cout << "  index=" << index-1
           << ", byte=" << int(byte) << " (" << bitset<8>(byte) << ")"
           << ", seven_bits=" << int(seven_bits)
           << ", seven_bits<<shift=" << (int(seven_bits) << (shift-7))
           << ", value=" << value 
           << ", has_next=" << has_next 
           << ", next_shift=" << shift << endl;
#endif
    }
    ProtoValue res;
    res.tag = tag;
    res.value = value;
    res.type = type;
    return res;
  }

  void parse(const char* buf, int size) {
    int index = 0;
    while (index < size) {
      ProtoValue v = parse_int(buf, index);
      switch (v.tag) {
      case TAG_CHANNELS:
        channels = v.value;
        break;
      
      case TAG_HEIGHT:
        height = v.value;
        break;
      
      case TAG_WIDTH:
        width = v.value;
        break;
        
      case TAG_DATA:
        image_bytes = v.value;
        data = buf + index;
        index += v.value;
        break;

      case TAG_LABEL:
        label = v.value;
        break;

      case TAG_ENCODED:
        encoded = v.value;
        break;
      }
    }
  }

  string verify() const {
    ostringstream s;
    if (channels == 0) s << " Field is not assigned: channels.";
    if (height == 0) s << " Field is not assigned: height.";
    if (width == 0) s << " Field is not assigned: width.";
    if (data == nullptr) s << " Field is not assigned: data.";
    if (label == 0) s << " Field is not assigned: label.";
    if (image_bytes == 0) s << " Field is not assigned: image_bytes.";
    return s.str();
  }

  string str() const {
    ostringstream s;
    s << "CHW: " << channels << "*" << height << "*" << width << ", "
      << "Bytes: " << image_bytes << ", "
      << "Label: " << label << ", "
      << "Encoded: " << (encoded ? "true": "false")
      << ".";
    return s.str();
  }
};


int main() {
  const char* mdb_path = getenv("CK_ENV_DATASET_IMAGENET_VAL_LMDB");
  int max_images = atoi(getenv("CK_IMG_COUNT"));
  cout << "Database path: " << mdb_path << endl;
  cout << "Images to read: " << max_images << endl;

  int last_error = MDB_SUCCESS;
  MDB_env* mdb_env = nullptr; // environment handle
  MDB_txn* mdb_txn = nullptr; // transaction handle
  MDB_dbi mdb_dbi; // database handle
  MDB_cursor* mdb_cursor = nullptr; // cursor handle
  
  try {
    // Create env
    last_error = mdb_env_create(&mdb_env);
    if (last_error != MDB_SUCCESS)
      throw runtime_error("Unable to create environment");
    cout << "Environment is created" << endl;


    // Open environment
    last_error = mdb_env_open(mdb_env, mdb_path, MDB_RDONLY | MDB_NOTLS | MDB_NOLOCK, 0664);
    if (last_error != MDB_SUCCESS)
      throw runtime_error("Unable to open environment");
    cout << "Environment is opend" << endl;


    // Open transaction
    // Transactions are always required, even for read-only access.
    last_error = mdb_txn_begin(mdb_env, nullptr, MDB_RDONLY, &mdb_txn);
    if (last_error != MDB_SUCCESS)
      throw runtime_error("Unable to open transaction");
    cout << "Transaction is opened" << endl;


    // Open database
    last_error = mdb_dbi_open(mdb_txn, nullptr, 0, &mdb_dbi);
    if (last_error != MDB_SUCCESS)
      throw runtime_error("Unable to open database");
    cout << "Database is opend" << endl;


    // Open cursor
    last_error = mdb_cursor_open(mdb_txn, mdb_dbi, &mdb_cursor);
    if (last_error != MDB_SUCCESS)
      throw runtime_error("Unable to open cursor");
    cout << "Cursor is opend" << endl;


    // List key-value pairs
    int index = 0;
    MDB_cursor_op cursor_op = MDB_FIRST;
    while (index < max_images) {
      cout << "---------  " << endl;
      MDB_val mdb_key, mdb_value;

      // Get first or next key-value pair
      last_error = mdb_cursor_get(mdb_cursor, &mdb_key, &mdb_value, cursor_op);
      
      // No more records in database
      if (last_error == MDB_NOTFOUND) {
        cout << "EOF" << endl;
        break;
      }

      if (last_error != MDB_SUCCESS)
        throw runtime_error("Unable to fetch key-value pair");

      // Key in Caffe ImageNet LMDB is source image file name
      string str_key = string(reinterpret_cast<const char*>(mdb_key.mv_data), mdb_key.mv_size);
      cout << "Key: " << str_key << ", value size: " << mdb_value.mv_size << endl;

      // Value is binary serialized Caffe Datum protobuf message
      CaffeDatum datum;
      datum.parse(reinterpret_cast<const char*>(mdb_value.mv_data), mdb_value.mv_size);
      cout << datum.str() << datum.verify() << endl;

      index++;
      cursor_op = MDB_NEXT;
    }
    cout << "---------  " << endl;
  }
  catch(const runtime_error& err) {
    cerr << "ERROR: " << err.what() << endl;
    cerr << "last_error = " << last_error << " ";
    switch (last_error) {
      case MDB_KEYEXIST: cerr << "(MDB_KEYEXIST) key/data pair already exists"; break;
      case MDB_NOTFOUND: cerr << "(MDB_NOTFOUND) key/data pair not found (EOF)"; break;
      case MDB_PAGE_NOTFOUND: cerr << "(MDB_PAGE_NOTFOUND) Requested page not found - this usually indicates corruption"; break;
      case MDB_CORRUPTED: cerr << "(MDB_CORRUPTED) Located page was wrong type"; break;
      case MDB_PANIC: cerr << "(MDB_PANIC) Update of meta page failed or environment had fatal error"; break;
      case MDB_VERSION_MISMATCH: cerr << "(MDB_VERSION_MISMATCH) Environment version mismatch"; break;
      case MDB_INVALID: cerr << "(MDB_INVALID) File is not a valid LMDB file"; break;
      case MDB_MAP_FULL: cerr << "(MDB_MAP_FULL) Environment mapsize reached"; break;
      case MDB_DBS_FULL: cerr << "(MDB_DBS_FULL) Environment maxdbs reached"; break;
      case MDB_READERS_FULL: cerr << "(MDB_READERS_FULL) Environment maxreaders reached"; break;
      case MDB_TLS_FULL: cerr << "(MDB_TLS_FULL) Too many TLS keys in use - Windows only"; break;
      case MDB_TXN_FULL: cerr << "(MDB_TXN_FULL) Txn has too many dirty pages"; break;
      case MDB_CURSOR_FULL: cerr << "(MDB_CURSOR_FULL) Cursor stack too deep - internal error"; break;
      case MDB_PAGE_FULL: cerr << "(MDB_PAGE_FULL) Page has not enough space - internal error"; break;
      case MDB_MAP_RESIZED: cerr << "(MDB_MAP_RESIZED) Database contents grew beyond environment mapsize"; break;
      case MDB_INCOMPATIBLE: cerr << "(MDB_INCOMPATIBLE) Operation and DB incompatible, or DB type changed"; break;
      case MDB_BAD_RSLOT: cerr << "(MDB_BAD_RSLOT) Invalid reuse of reader locktable slot"; break;
      case MDB_BAD_TXN: cerr << "(MDB_BAD_TXN) Transaction must abort, has a child, or is invalid"; break;
      case MDB_BAD_VALSIZE: cerr << "(MDB_BAD_VALSIZE) Unsupported size of key/DB name/data, or wrong DUPFIXED size"; break;
      case MDB_BAD_DBI: cerr << "(MDB_BAD_DBI) The specified DBI was changed unexpectedly"; break;
      default: cerr << "";
    }
    cerr << endl;
  }

  // Close cursor
  if (mdb_cursor) {
    mdb_cursor_close(mdb_cursor);
    cout << "Cursor is closed" << endl;
  }

  // Closing a database handle is not necessary.
  // If the transaction is aborted the handle will be closed automatically.
  // After a successful commit the handle will reside in the shared environment,
  // and may be used by other transactions.

  // Close transaction
  if (mdb_txn) {
    mdb_txn_abort(mdb_txn);
    cout << "Transaction is closed" << endl;
  }

  // Close environment
  if (mdb_env) {
    mdb_env_close(mdb_env);
    cout << "Environment is closed" << endl;
  }
 
  return 0;
}

#include <iostream>
#include <fstream>
#include <cstdlib>

#include "lmdb.h" 

using namespace std;

struct Value {
  int tag;
  int value;
  int type;
};

Value parse_int(const char* buf, int& index) {
  char varint_key = buf[index++];
  int wire_type = int(varint_key & 0x7);
  int value_tag = int((varint_key & ~0x7) >> 3);
  cout << "tag = " << value_tag << ", wire_type = " << wire_type << endl;
  char byte = buf[index++];
  int value = (byte & ~(0x8<<4));
  // when most significat bit is set
  // it means there is another byte
  bool has_next = byte & (0x8<<4);
  int shift_bits = 8;
  while (has_next) {
    byte = buf[index++];
    has_next = byte & (0x8<<4);
    value = value + (byte << shift_bits);
    shift_bits += 8;
  }
  Value res;
  res.tag = value_tag;
  res.value = value;
  res.type = wire_type;
  return res;
}

void get_CHW(Value v, int& c, int& h, int& w) {
  switch (v.tag) {
    case 1:
      if (c != 0)
        throw runtime_error("Invalid data, channels value already set");
      c = v.value;
      break;
      
    case 2:
      if (h != 0)
        throw runtime_error("Invalid data, heights value already set");
      h = v.value;
      break;
      
    case 3:
      if (w != 0)
        throw runtime_error("Invalid data, width value already set");
      w = v.value;
      break;
  }
}

void process_key_value(MDB_val& mdb_key, MDB_val& mdb_value) {
  int key_size = mdb_key.mv_size;
  int data_size = mdb_value.mv_size;
  const char* key_buf = static_cast<const char*>(mdb_key.mv_data);
  const char* value_buf = static_cast<const char*>(mdb_value.mv_data);

  string key_str = string(key_buf, key_size);

  cout << "Key: " << key_str << ", Value size: " << data_size << endl;

  int index = 0;

  int C = 0, H = 0, W = 0;
  Value r1 = parse_int(value_buf, index); get_CHW(r1, C, H, W);
  Value r2 = parse_int(value_buf, index); get_CHW(r2, C, H, W);
  Value r3 = parse_int(value_buf, index); get_CHW(r3, C, H, W);
  cout << "Image dims (CHW): " << C << ", " << H << ", " << W << endl;

  Value r4 = parse_int(value_buf, index);
  cout << "Image bytes: " << r4.value << endl;
}

int main() {
  const char* mdb_path = getenv("CK_ENV_DATASET_IMAGENET_VAL_LMDB");
  cout << "Database path: " << mdb_path << endl;

  int last_error = 0;
  MDB_env* mdb_env = nullptr; // environment handle
  MDB_txn* mdb_txn = nullptr; // transaction handle
  MDB_dbi mdb_dbi; // database handle
  MDB_cursor* mdb_cursor; // cursor handle
  MDB_val mdb_key;
  MDB_val mdb_value;
  
  try {
    
    // Create env
    last_error = mdb_env_create(&mdb_env);
    if (last_error != 0)
      throw runtime_error("Unable to create environment");
    cout << "Environment is created" << endl;


    // Open environment
    last_error = mdb_env_open(mdb_env, mdb_path, MDB_RDONLY | MDB_NOTLS | MDB_NOLOCK, 0664);
    if (last_error != 0)
      throw runtime_error("Unable to open environment");
    cout << "Environment is opend" << endl;


    // Open transaction
    // Transactions are always required, even for read-only access.
    last_error = mdb_txn_begin(mdb_env, nullptr, MDB_RDONLY, &mdb_txn);
    if (last_error != 0)
      throw runtime_error("Unable to open transaction");
    cout << "Transaction is opened" << endl;


    // Open database
    last_error = mdb_dbi_open(mdb_txn, nullptr, 0, &mdb_dbi);
    if (last_error != 0)
      throw runtime_error("Unable to open database");
    cout << "Database is opend" << endl;


    // Open cursor
    last_error = mdb_cursor_open(mdb_txn, mdb_dbi, &mdb_cursor);
    if (last_error != 0)
      throw runtime_error("Unable to open cursor");
    cout << "Cursor is opend" << endl;


    // Get first key-value pair
    last_error = mdb_cursor_get(mdb_cursor, &mdb_key, &mdb_value, MDB_FIRST);
    if (last_error != 0)
      throw runtime_error("Unable to fetch first key-value pair");
    process_key_value(mdb_key, mdb_value);


    // Get key-value pairs
    int index = 1;
    while (true) {
      last_error = mdb_cursor_get(mdb_cursor, &mdb_key, &mdb_value, MDB_NEXT);
      if (last_error == MDB_NOTFOUND) {
        cout << "EOF" << endl;
        break;
      }
      if (last_error != 0)
        throw runtime_error("Unable to fetch first key-value pair");

      process_key_value(mdb_key, mdb_value);

      index++;
      if (index == 40)
        break;
    }
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

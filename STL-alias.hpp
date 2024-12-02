template<typename K, typename V, typename C = std::less<K>,
		 typename A = std::allocator<std::pair<const K, V> > >
using map_t = std::map<K, V, C, A>;

template<typename K, typename C = std::less<K>, typename A = std::allocator<K> >
using set_t = std::set<K, C, A>;

template<typename T, typename A = std::allocator<T> >
using vct_t = std::vector<T, A>;

namespace fs = std::filesystem;
using path_t = fs::path;

using abool_t = std::atomic_bool;
using ai32_t = std::atomic_int32_t;
using ai64_t = std::atomic_int64_t;
using aptid_t = std::atomic<pthread_t>;
using atime_t = std::atomic<SysTime_t>;

using str_t = std::string;
using StrSet_t = set_t<str_t>;

using ifs_t = std::ifstream;
using ofs_t = std::ofstream;
using oss_t = std::ostringstream;
using ost_t = std::ostream;
using stv_t = std::string_view;
using wstr_t = std::wstring;

using namespace leon_utl;

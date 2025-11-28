#ifndef RELATIONS_H
#define RELATIONS_H

#include "../bugtypes.h"

namespace Db::Relations {

IntListResult get(int id);
QString make(int id1, int id2);
QString remove(int id1, int id2);

} // namespace Db::Relations

#endif // RELATIONS_H

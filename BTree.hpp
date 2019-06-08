//
// Created by 17961 on 2019/6/8.
//

#ifndef UNTITLED_BTREE_HPP
#define UNTITLED_BTREE_HPP
#include "utility.hpp"
#include <functional>
#include <cstddef>
#include <map>
#include <fstream>
#include <stdio.h>

const int size_M = 500;
const int size_L = 250;

using namespace std;

namespace sjtu {
    typedef size_t position;

    template <class Key, class Value, class Compare = std::less<Key> >
    class BTree {
    public:
        typedef pair<Key, Value> value_type;
        struct Treenode {
            int current_size;
            position thisgood;
            position parent;
            bool rank;//判断孩子节点是否是叶子0为孩子是叶子1为孩子是节点
            position child[size_M + 1];
            Key keylist[size_M + 1];
            Treenode() {
                thisgood = 0;
                parent = 0;

                for (int i = 0; i < current_size; ++i) child[i] = 0;
            }
        };
        struct dataNode {
            position thisgood;
            position parent, leftgood, rightgood;
            int current_size;
            value_type data[size_L + 1];
            dataNode() {
                thisgood = 0;
                parent = 0;
                leftgood = 0;
                rightgood = 0;
            }
        };
        struct Basicthing {
            position head;
            position root;
            position tail;
            position end;

            int Treesize;
            int size;
            Basicthing() {
                head = 0;
                root = 0;
                tail = 0;
            }
        };
        FILE *file;

        Basicthing bas;
        // Your private members go here

        void writeNode(void *thing, size_t offset, size_t size, size_t num) {
            if (fseek(file, offset, 0)) throw"文件打开失败！73";
            fwrite(thing, size, num, file);
            fflush(file);
        }
        // Default Constructor and Copy Constructor
        BTree() {
            file=fopen("123.txt", "rb+");

            Treenode Root;
            dataNode Head;
            bas.Treesize = 0;
            Root.thisgood = bas.root = sizeof(Basicthing);
            bas.head = Head.thisgood = sizeof(Basicthing) + sizeof(Treenode);
            bas.tail = sizeof(Basicthing) + sizeof(Treenode) + sizeof(dataNode);
            bas.end = bas.tail;
            Root.parent = 0;
            Root.current_size = 1; Head.current_size = 0;
            Root.child[0] = Head.thisgood;
            Head.parent = Root.thisgood;
            Root.rank = 0;//孩子是叶子
            writeNode(&bas, 0, sizeof(Basicthing), 1);
            writeNode(&Root, Root.thisgood, sizeof(Treenode), 1);
            writeNode(&Head, Head.thisgood, sizeof(dataNode), 1);
            // Todo Default
        }

        ~BTree() {

            // Todo Destructor
        }
        // Insert: Insert certain Key-Value into the database
        // Return a pair, the first of the pair is the iterator point to the new
        // element, the second of the pair is Success if it is successfully inserted

        OperationResult insertdata(const Key key,const Value value) {
            Treenode tmp;
            bas.Treesize++;
            if (fseek(file, bas.root, 0)) throw "偏移失败118";

            fread(&tmp, sizeof(Treenode), 1, file);
            while (tmp.rank) {
                int i;
                for (i = 0; i < tmp.current_size; ++i) if (key < tmp.keylist[i]) break;
                if (fseek(file, tmp.child[i], 0)) throw"偏移失败124";
                fread(&tmp, sizeof(Treenode), 1, file);
            }
            int i;
            for (i = 0; i < tmp.current_size; ++i) if (key < tmp.keylist[i]) break;
            int getsonhere = i;
            dataNode tmpdata;
            if (fseek(file, tmp.child[i], 0))

                throw"偏移失败130";

            fread(&tmpdata, sizeof(dataNode), 1, file);
            for (i = 0; i < tmpdata.current_size; ++i) {
                if (key == tmpdata.data[i].first) return Fail;
                if (key < tmpdata.data[i].first) break;
            }
            for (int j = tmpdata.current_size; j > i + 1; --j) {tmpdata.data[j].first = tmpdata.data[j - 1].first;
            tmpdata.data[j].second = tmpdata.data[j - 1].second;}
            tmpdata.data[i].first = key;
            tmpdata.data[i].second = value;
            tmpdata.current_size++;
            writeNode(&tmpdata, tmpdata.thisgood, sizeof(dataNode), 1);
            if (tmpdata.current_size > size_L) {
                splitleaf(tmpdata, tmp, getsonhere);
                if (fseek(file, tmpdata.parent, 0))  throw"文件打开失败！151";
                fread(&tmp, sizeof(Treenode), 1, file);
                if (tmp.current_size > size_M) {
                    while (tmp.parent != 0) {
                        splitnode(tmp);
                        if(fseek(file, tmp.parent, 0)) throw "文件pianyi错误";
                        fread(&tmp, sizeof(Treenode), 1, file);
                        if (tmp.current_size <= size_M) break;
                    }
                    if (tmp.current_size > size_M) splitRoot(tmp);
                }

            }
            return Success;
        }
        void splitnode(Treenode &tmpnode) {
            Treenode newnode;

            int num = tmpnode.current_size;
            newnode.rank = tmpnode.rank;
            newnode.current_size = tmpnode.current_size - (tmpnode.current_size >> 1);
            tmpnode.current_size = tmpnode.current_size >> 1;
            newnode.parent = tmpnode.parent;
            for (int i = tmpnode.current_size; i < num; ++i) {
                newnode.keylist[i - tmpnode.current_size] = tmpnode.keylist[i];
                newnode.child[i - tmpnode.current_size] = tmpnode.child[i];
            }
            newnode.thisgood = bas.end;
            bas.end += sizeof(Treenode);
            writeNode(&tmpnode, tmpnode.thisgood, sizeof(Treenode), 1);
            writeNode(&newnode, newnode.thisgood, sizeof(Treenode), 1);
            Treenode Treeson;
            dataNode datason;
            /*
            为儿子们赋值父亲节点地址
            */
            for (int i = 0; i < newnode.current_size; ++i) {
                if (fseek(file, newnode.child[i], 0)) throw"偏移失败183";
                if (tmpnode.rank) {
                    fread(&Treeson, sizeof(Treenode), 1, file);
                    Treeson.parent = newnode.thisgood;
                    writeNode(&Treeson, newnode.child[i], sizeof(Treenode), 1);
                }
                else {
                    fread(&datason, sizeof(dataNode), 1, file);
                    datason.parent = newnode.thisgood;
                    writeNode(&datason, newnode.child[i], sizeof(dataNode), 1);
                }
            }
            Treenode par;
            if (fseek(file, tmpnode.parent, 0)) throw"偏移失败189";
            fread(&par, sizeof(Treenode), 1, file);
            int posi = findNode(par, tmpnode.keylist[0]);
            for (int i = par.current_size; i > posi + 1; --i) {
                par.keylist[i] = par.keylist[i - 1];
                par.child[i] = par.child[i - 1];
            }
            par.keylist[posi + 1] = newnode.keylist[0];
            par.child[posi + 1] = newnode.thisgood;
            writeNode(&par, par.thisgood, sizeof(Treenode), 1);
        }
        void splitRoot(Treenode &root) {
            Treenode newRoot;
            Treenode newnode;
            int num;
            newRoot.rank = 1;
            newRoot.parent = 0;
            newRoot.thisgood = bas.root = bas.end;
            //writeNode(&newRoot, bas.end, sizeof(Treenode), file);
            bas.end += sizeof(Treenode);
            newnode.thisgood = bas.end;
            bas.end += sizeof(Treenode);
            num = root.current_size;
            newnode.rank = root.rank;
            newnode.current_size = root.current_size - (root.current_size >> 1);
            root.current_size = root.current_size >> 1;
            for (int i = root.current_size; i < num; ++i) {
                newnode.keylist[i - root.current_size] = root.keylist[i];
                newnode.child[i - root.current_size] = root.child[i];
            }
            newnode.parent = root.parent = bas.root;
            newRoot.keylist[0] = root.keylist[0];
            newRoot.child[0] = root.thisgood;
            newRoot.keylist[1] = newnode.keylist[0];
            newRoot.child[1] = newnode.thisgood;
            writeNode(&newRoot, newRoot.thisgood, sizeof(Treenode), 1);
            writeNode(&newnode, newnode.thisgood, sizeof(Treenode), 1);

        }
        void splitleaf(dataNode &tmpdata, Treenode &tmp, const int sonposi) {
            dataNode newleaf;
            Key newkey;
            int num = tmpdata.current_size;
            newleaf.current_size = tmpdata.current_size - (tmpdata.current_size >> 1);
            tmpdata.current_size = tmpdata.current_size >> 1;
            for (int i = tmpdata.current_size; i < num; ++i) {tmpdata.data[i].first = tmpdata.data[i - 1].first;
                tmpdata.data[i].second = tmpdata.data[i - 1].second;}
            newkey = newleaf.data[0].first;
            newleaf.parent = tmpdata.parent;
            newleaf.thisgood = bas.end;
            writeNode(&tmpdata, tmpdata.thisgood, sizeof(dataNode), 1);
            writeNode(&newleaf, bas.end, sizeof(dataNode), 1);
            for (int i = tmp.current_size - 1; i > sonposi; --i) {
                tmp.child[i + 1] = tmp.child[i];
                tmp.keylist[i + 1] = tmp.keylist[i];
            }
            tmp.child[sonposi + 1] = newleaf.thisgood;
            tmp.keylist[sonposi + 1] = newkey;
            tmp.current_size++;
            writeNode(&tmp, tmp.thisgood, sizeof(Treenode), 1);
            bas.end += sizeof(dataNode);
        }
        OperationResult insert(const Key& key, const Value& value) {
            return insertdata(key, value);
        }
        int findNode(Treenode &par, Key ke) {
            int i;
            for (i = 0; i < par.current_size; ++i) if (ke == par.keylist[i]) return i;
        }
        // Erase: Erase the Key-Value
        // Return Success if it is successfully erased
        // Return Fail if the key doesn't exist in the database
        OperationResult erase(const Key& key) {
            // TODO erase function
            return Fail;  // If you can't finish erase part, just remaining here.
        }

        // Check whether this BTree is empty
        bool empty() const { return bas.Treesize == 0; }
        // Return the number of <K,V> pairs
        size_t size() const {}
        // Clear the BTree
        void clear() {}
        // Return the value refer to the Key(key)
        Value at(const Key& key) {
            Treenode tmp;

            if (fseek(file, bas.root, 0)) {

                throw "偏移失败287";
            }
            fread(&tmp, sizeof(Treenode), 1, file);
            while (tmp.rank) {
                int i;
                for (i = 0; i < tmp.current_size; ++i) if (key < tmp.keylist[i]) break;
                if (fseek(file, tmp.child[i], 0))

                    throw"偏移失败295";

                fread(&tmp, sizeof(Treenode), 1, file);
            }
            int i;
            for (i = 0; i < tmp.current_size; ++i) if (key == tmp.keylist[i]) break;
            return tmp.keylist[i].second;
        }
        /**
         * Returns the number of elements with key
         *   that compares equivalent to the specified argument,
         * The default method of check the equivalence is !(a < b || b > a)
         */
        size_t count(const Key& key) const {}
        /**
         * Finds an element with key equivalent to key.
         * key value of the element to search for.
         * Iterator to an element with key equivalent to key.
         *   If no such element is found, past-the-end (see end()) iterator is
         * returned.
         */
    };
}  // namespace sjtu

#endif
 UNTITLED_BTREE_HPP

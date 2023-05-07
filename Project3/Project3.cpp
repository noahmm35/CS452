#include <iostream>

using namespace std;

class Node
{
    private:
        Node * parent_;
        int value_[3];
        Node * child_[4]; //4th child to allow overflow of nodes

    public:
        Node(int val)
        {
            parent_ = nullptr;

            for(int i=0; i<4; i++)
                child_[i] = nullptr;

            value_[0] = -1;
            value_[1] = -1;
            value_[2] = val;
        }

        Node()
        {
            parent_ = nullptr;

            for(int i=0; i<4; i++) {
                child_[i] = nullptr;
                if(i!=4)
                    value_[i] = -1;
            }
        }

        ~Node()
        {
            if(child_[0]!=nullptr)
                delete child_[0];
            if(child_[1]!=nullptr)
                delete child_[1];
            if(child_[2]!=nullptr)
                delete child_[2];
            if(child_[3]!=nullptr)
                delete child_[3];
        }

        //Helpers
        int numChildren()
        {
            int c = 0;
            if(child_[0] != nullptr)
                c++;
            if(child_[1] != nullptr)
                c++;
            if(child_[2] != nullptr)
                c++;
            if(child_[3] != nullptr)
                c++;
            return c;
        }

        Node * getLeftChild() { return child_[0]; }
        Node * getMiddleChild() { return child_[1]; }
        Node * getRightChild() { return child_[2]; }
        Node * getParent() { return parent_; }
        Node * getOverflow() { return child_[3]; }

        int getLeftValue() { return value_[0]; }
        int getMiddleValue() { return value_[1]; }
        int getRightValue() { return value_[2]; }

        void setParent(Node * p) 
        { 
            if(p!=nullptr)
                parent_ = p;
        }

        void setLeftChild(Node * l) 
        { 
            child_[0] = l; 
            if(l!=nullptr)
                l->setParent(this); 
        }

        void setMiddleChild(Node * m) 
        { 
            child_[1] = m; 
            if(m!=nullptr)
                m->setParent(this); 
        }

        void setRightChild(Node * r) 
        {
            child_[2] = r; 
            if(r!=nullptr)
                r->setParent(this); 
        }

        void setOverflow(Node * o) 
        {
            child_[3] = o; 
            if(o!=nullptr)
                o->setParent(this); 
        }

        void setLeftValue(int v) { value_[0] = v; }
        void setRightValue(int v) { value_[2] = v; }
        void setMiddleValue(int v) { value_[1] = v; }
};

class Tree
{
    private:
        Node * root_;
        Node * inf = new Node(2000);
    
    public:
        Tree()
        {
            root_ = nullptr;
        }

        Tree(Node*root)
        {
            root_ = root;
        }

        ~Tree()
        {
            if(root_!=nullptr)
                delete root_;

            delete inf;
        }

        void setRoot(Node * root) { root_ = root; }

        Node * getRoot() { return root_; }

        void updateValues(Node * start)
        {
            if(start->numChildren()==0)
                return;
            
            if(start->getLeftChild()!=nullptr)
            {
                updateValues(start->getLeftChild());
                start->setLeftValue(start->getLeftChild()->getRightValue());
            }

            if(start->getRightChild()!=nullptr)
            {
                updateValues(start->getRightChild());
            start->setRightValue(start->getRightChild()->getRightValue());
            }

            if(start->getMiddleChild()!=nullptr)
            {
                updateValues(start->getMiddleChild());
                start->setMiddleValue(start->getMiddleChild()->getRightValue());
            }
        }

        Node * Search(Node * root, int valToFind)
        {
            if(root->numChildren() == 0) //root has no children
                return root;

            if(valToFind <= root->getLeftValue())
                return Search(root->getLeftChild(), valToFind);
            else if(root->getMiddleChild()!=nullptr && valToFind <= root->getMiddleValue())
                return Search(root->getMiddleChild(), valToFind);
            else if(valToFind <= root->getRightValue())
                return Search(root->getRightChild(), valToFind);
            else //root is larger than largest element in array
                return inf;
        }

        void Insert(int valToAdd)
        {
            Node * newNode = new Node(valToAdd);
            Node * succ = Search(root_, valToAdd);
            if (newNode == succ)
                return;
            else
                Absorb(newNode, succ);
        }

        void Absorb(Node * newChild, Node * successor)
        {
            if(successor == root_)
            {
                Node * MegaRoot = new Node();
                setRoot(MegaRoot);
                MegaRoot->setLeftChild(newChild);
                MegaRoot->setRightChild(successor);

                updateValues(MegaRoot);
                return;
            }

            Node * parent = successor->getParent();

            if(parent->numChildren() < 3)
            {   
                // easy insertion, 3 cases
                if(newChild->getRightValue() < parent->getLeftValue()) // newChild goes in left
                {
                    parent->setMiddleChild(parent->getLeftChild());
                    parent->setLeftChild(newChild);
                }
                else if(newChild->getRightValue() > parent->getRightValue()) // newChild goes in right **
                {
                    parent->setMiddleChild(parent->getRightChild());
                    parent->setRightChild(newChild);
                }
                else                                                        // newChild goes in middle
                {
                    parent->setMiddleChild(newChild);
                }

                updateValues(parent);
            }
            else
            {
                //overfill parent by adding a
                parent->setOverflow(newChild);

                //create a p-left and move two smallest over
                Node * pLeft = new Node();
                int pos = overflowPosition(parent);

                if(pos == 1)
                {
                    pLeft->setLeftChild(parent->getOverflow());
                    pLeft->setRightChild(parent->getLeftChild());

                    parent->setLeftChild(parent->getMiddleChild());
                    
                    parent->getMiddleChild()->setParent(nullptr);
                    parent->setMiddleChild(nullptr);
                    parent->setMiddleValue(-1);
                }
                else if(pos == 2)
                {
                    pLeft->setLeftChild(parent->getLeftChild());
                    pLeft->setRightChild(parent->getOverflow());

                    parent->setLeftChild(parent->getMiddleChild());

                    parent->getMiddleChild()->setParent(nullptr);
                    parent->setMiddleChild(nullptr);
                    parent->setMiddleValue(-1);

                }
                else if(pos == 3)
                {
                    pLeft->setLeftChild(parent->getLeftChild());
                    pLeft->setRightChild(parent->getMiddleChild());

                    parent->setLeftChild(parent->getOverflow());

                    parent->getMiddleChild()->setParent(nullptr);
                    parent->setMiddleChild(nullptr);
                    parent->setMiddleValue(-1);
                }
                else
                {
                    pLeft->setLeftChild(parent->getLeftChild());
                    pLeft->setRightChild(parent->getMiddleChild());

                    parent->setLeftChild(parent->getRightChild());
                    parent->setRightChild(parent->getOverflow());
                    
                    parent->getMiddleChild()->setParent(nullptr);
                    parent->setMiddleChild(nullptr);
                    parent->setMiddleValue(-1);
                }

                parent->getOverflow()->setParent(nullptr);
                parent->setOverflow(nullptr);

                updateValues(parent);
                updateValues(pLeft);


                Absorb(pLeft, parent);
            }
        }

        void Delete(int valToKill)
        {
            Node *a = Search(root_, valToKill);

            if (a->getRightValue() == valToKill)
                Discard(a);
        }

        void Discard(Node * removeChild)
        {
            int place=0;
            Node * parent = removeChild->getParent();

            //remove connection btwn parent and remove child
            removeChild->setParent(nullptr);

            if(removeChild == parent->getLeftChild())
            {
                parent->setLeftChild(nullptr);
                parent->setLeftValue(-1);
                place=0;
            }
            else if (removeChild == parent->getMiddleChild())
            {
                parent->setMiddleChild(nullptr);
                parent->setMiddleValue(-1);
                place=1;
            }
            else
            {
                parent->setRightChild(nullptr);
                parent->setRightValue(-1);
                place=2;
            }

            if(parent->numChildren() == 2)
            {
                //might need to readjust parent node based on child placement
                if(place==0)
                    parent->setMiddleChild(parent->getLeftChild());
                else if(place==2)
                    parent->setRightChild(parent->getMiddleChild());
                else
                    return;

                parent->setMiddleChild(nullptr);
                parent->setMiddleValue(-1);

                return;
            }

            //totalChildren amongst entire generation (p+ siblings)
            int totalChildren = parent->numChildren();

            if(totalChildren >= 4) //no recursion
            {
                //distribute children among at least 2 parents
                //may have to remove a parent in case of 5 children
                //might have to update naviagtion info

                return;
            }

            // let b be the ONLY child of parent
            if(parent == root_)
            {
                // make b the root of tree
                // delete parent

                return;
                // ** Tree is not necessarily 2-3 tree?
            }

            //allocate b to sibling of p
            return Discard(parent);
        }
        
        void Print(Node * root)
        {
            //preorder-ish traversal
            if(root == nullptr)
                return;

            if(root->getLeftValue() != -1)
                cout << root->getLeftValue() << ",";

            if(root->getMiddleValue() != -1)
                cout << root->getMiddleValue() << ",";
            else if (root->getMiddleValue() == -1 && root->numChildren()>0)
                cout << "-,";
            
            if(root->getRightValue() != -1)
                cout << root->getRightValue();
            
            cout << endl;

            Print(root->getLeftChild());
            if(root->getMiddleChild()!=nullptr)
                Print(root->getMiddleChild());
            Print(root->getRightChild());
        }

        int overflowPosition(Node * n)
        {
            int pos=0, oVal=n->getOverflow()->getRightValue();

            if(oVal < n->getLeftValue())
                return 1;
            else if (oVal > n->getLeftValue() && oVal < n->getMiddleValue())
                return 2;
            else if(oVal > n->getMiddleValue() && oVal < n->getRightValue())
                return 3;
            else
                return 4;
        }
};

int main()
{
    //manually build tree from uzi... oof
    Node * root = new Node(39);

    Tree tree(root);
    int leaves[14] = {17,28,3,21,33,1,8,5,11,14,19,24,31,36};

    for(int i=0;i<14;i++)
        tree.Insert(leaves[i]);

    tree.Print(tree.getRoot());

    cout << "\nDeleting 14\n\n";
    tree.Delete(14);

    tree.Print(tree.getRoot());

    return 0;
}

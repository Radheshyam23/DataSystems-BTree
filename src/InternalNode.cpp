#include "InternalNode.hpp"

//creates internal node pointed to by tree_ptr or creates a new one
InternalNode::InternalNode(const TreePtr &tree_ptr) : TreeNode(INTERNAL, tree_ptr) {
    this->keys.clear();
    this->tree_pointers.clear();
    if (!is_null(tree_ptr))
        this->load();
}

//max element from tree rooted at this node
Key InternalNode::max() {
    Key max_key = DELETE_MARKER;
    TreeNode* last_tree_node = TreeNode::tree_node_factory(this->tree_pointers[this->size - 1]);
    max_key = last_tree_node->max();
    delete last_tree_node;
    return max_key;
}

//if internal node contains a single child, it is returned
TreePtr InternalNode::single_child_ptr() {
    if (this->size == 1)
        return this->tree_pointers[0];
    return NULL_PTR;
}

//inserts <key, record_ptr> into subtree rooted at this node.
//returns pointer to split node if exists
TreePtr InternalNode::insert_key(const Key &key, const RecordPtr &record_ptr) 
{
    // For DEBUG
    // cout<<"Entered InternalNode::insert_key(), key: "<<key<<endl;
    // cout<<"Current Internal Node:\n";
    // for (int i=0; i<this->size; i++)
    //     cout<<this->keys[i]<<" ";
    // cout<<endl;
    

    // The first step is to find the position for the new key
    int posn = 0;

    // break when we find the first DELETE_MARKER or when we find a key that is greater than the key we are inserting
    for (; posn < this->size - 1; posn++)
    {
        if (this->keys[posn] == DELETE_MARKER)
            break;

        if (key <= this->keys[posn])
            break;
    }

    // cout<<"Position: "<<posn<<endl;

    auto child_node = TreeNode::tree_node_factory(this->tree_pointers[posn]);

    // the potential_split_child_node_ptr is the pointer to the new leaf that is created
    // but remember that the data_pointers is of the form:
    // <P1,K1,P2,K2,P3>
    // So this particular leaf will have a pointer, but we wont add any new key for it.
    // we will only update the key of the previous one.
    TreePtr split_child_ptr = child_node->insert_key(key, record_ptr);
    auto split_child_node = TreeNode::tree_node_factory(split_child_ptr);

    if(is_null(split_child_ptr)) 
    {
        // cout<<"No split. Key added in leaf successfully\n";
        return NULL_PTR;
    }
    
    // cout<<"Split of leaf node required\n";

    // Inserting the new node in the position posn
    vector<TreePtr> temp;
    for (int i=0; i<=posn; i++)
        temp.push_back(this->tree_pointers[i]);
    temp.push_back(split_child_ptr);
    for (int i=posn+1; i<this->size; i++)
        temp.push_back(this->tree_pointers[i]);
    this->size++;
    this->tree_pointers = temp;

    // update the original key value at posn in the internal node
    this->keys[posn] = child_node->max();
    this->keys.insert(this->keys.begin()+posn+1, split_child_node->max());

    // cout<<"Split occurred in internal node at: "<<child_node->max()<<" at posn: "<<posn<<endl;

    if (!this->overflows())
    {
        // cout<<"No overflow of internal node\n";
        this->dump();
        return NULL_PTR;
    }

    // cout<<"Overflow. Split of internal node required\n";

    // if overflow occurs, we need to split the node
    // we will split the node into two nodes, and return the pointer to the new node
    auto new_internal_node = new InternalNode();
    auto new_internal_node_ptr = new_internal_node->tree_ptr;

    // Now we need to split the data_pointers between the two nodes
    int mid = MIN_OCCUPANCY;

    for (int i=mid; i<this->size-1; i++)
    {
        new_internal_node->tree_pointers.push_back(this->tree_pointers[i]);
        new_internal_node->keys.push_back(this->keys[i]);
        new_internal_node->size++;
    }
    new_internal_node->tree_pointers.push_back(this->tree_pointers[this->size-1]);
    new_internal_node->size++;

    // cout<<"Split at: "<<this->keys[mid-1]<<endl;

    // delete those shifted keys from the original node
    // delete entries from the vector
    this->tree_pointers.erase(this->tree_pointers.begin()+mid+1, this->tree_pointers.end());
    this->keys.erase(this->keys.begin()+mid, this->keys.end());
    this->size = mid;

    // Sample representation to remember the structure:
    // W 1 X 2 Y 3 Z
    // W 1 X
    //  

    // [W X Y Z]        (Tree Pointers)
    // [1 2 3]          (Keys)

    // W X Y
    // 1 2

    // Y Z
    // 3 

    new_internal_node->dump();
    this->dump();
    return new_internal_node_ptr;
}


void InternalNode::redistributeData(TreeNode *from, TreeNode *to, int fromIndex, int toIndex)
{
    // cout<<"Entered InternalNode::redistributeData()"<<endl;
    auto fromInternal = dynamic_cast<InternalNode*>(from);
    auto toInternal = dynamic_cast<InternalNode*>(to);
    if (fromIndex < toIndex)
    {
        // cout<<"LeftRedistribute"<<endl;
        // that means we need to move the last dataentry from from to the beginning of to

        toInternal->tree_pointers.insert(toInternal->tree_pointers.begin(), fromInternal->tree_pointers.back());
        toInternal->keys.insert(toInternal->keys.begin(), fromInternal->keys.back());
        toInternal->size++;

        // cout<<"Pushed ptr and key"<<endl;

        fromInternal->tree_pointers.pop_back();
        fromInternal->keys.pop_back();
        fromInternal->size--;

        // cout<<"Popped ptr and key"<<endl;

        fromInternal->dump();
        toInternal->dump();

        // cout<<"Returning..."<<endl;
    }
    else
    {
        // cout<<"RightRedistribute"<<endl;
        // that means we need to move the first dataentry from from to the end of to

        toInternal->tree_pointers.push_back(fromInternal->tree_pointers.front());
        toInternal->keys.push_back(fromInternal->keys.front());
        toInternal->size++;

        // cout<<"Pushed ptr and key"<<endl;

        fromInternal->tree_pointers.erase(fromInternal->tree_pointers.begin());
        fromInternal->keys.erase(fromInternal->keys.begin());
        fromInternal->size--;

        // cout<<"Popped ptr and key"<<endl;

        fromInternal->dump();
        toInternal->dump();

        // cout<<"Returning..."<<endl;
    }
}

// merge sibling node with itself and delete sibling node
void InternalNode::merge_nodes(TreeNode* sibling_tree_node) 
{
    // cout<<"Entered InternalNode::merge_node()"<<endl;
    InternalNode* sibling_node = dynamic_cast<InternalNode*>(sibling_tree_node);
    auto currLast = TreeNode::tree_node_factory(this->tree_pointers.back());
    
    this->keys.push_back(currLast->max());

    if (this->tree_pointers.back() != sibling_node->tree_pointers.front())
        this->tree_pointers.insert(this->tree_pointers.end(), sibling_node->tree_pointers.begin(), sibling_node->tree_pointers.end());
    else
        this->tree_pointers.insert(this->tree_pointers.end(), sibling_node->tree_pointers.begin()+1, sibling_node->tree_pointers.end());

    this->keys.insert(this->keys.end(), sibling_node->keys.begin(), sibling_node->keys.end());

    this->size = this->keys.size() + 1;

    sibling_node->delete_node();
    this->dump();
}

//deletes key from subtree rooted at this if exists
void InternalNode::delete_key(const Key &key) 
{
    // For DEBUG
    // cout<<"Entered InternalNode::delete_key, key:"<<key<<endl;
    // cout<<"The internal node has the following:\n";
    // for (int i=0; i<this->size; i++)
    //     cout<<this->keys[i]<<" ";
    // cout<<endl;

    // The first step is to find the position for the new key
    int posn = 0;

    // break when we find the first DELETE_MARKER or when we find a key that is greater than the key we are inserting
    for (; posn < this->size - 1; posn++)
    {
        if (this->keys[posn] == DELETE_MARKER)
            break;

        if (key <= this->keys[posn])
            break;
    }
    // cout<<"Position: "<<posn<<endl;

    auto child_node = TreeNode::tree_node_factory(this->tree_pointers[posn]);
    child_node->delete_key(key);

    // update the key value at posn in the internal node
    // the last pointer ofc doesnt have an associated key
    if (posn != this->size-1)
    {
        // cout<<"Updating key at posn from "<<this->keys[posn]<<" to "<<child_node->max()<<endl;
        this->keys[posn] = child_node->max();
    }


    if (!child_node->underflows())
    {
        // cout<<"No underflow"<<endl;
        this->dump();
        return;
    }

    // order specified:
    // left redistribute > left merge > right redistribute > right merge
    
    bool leftRedistribute = false, rightRedistribute = false;
    if (posn != 0)
    {
        // cout<<"Left Neighbour exists"<<endl;
        auto left_neighbour = TreeNode::tree_node_factory(this->tree_pointers[posn-1]);
        if (child_node->size + left_neighbour->size >= 2*MIN_OCCUPANCY)
        {
            // left redistribution possible!
            // cout<<"Left redistribution possible"<<endl;
            left_neighbour->redistributeData(left_neighbour, child_node, posn-1, posn);
            this->keys[posn-1] = left_neighbour->max();
            leftRedistribute = true;
            this->dump();
            return;
        }
    }
    // left merge
    if (posn != 0)
    {
        // cout<<"Left Neighbour exists. Performing Left Merge"<<endl;
        auto left_neighbour = TreeNode::tree_node_factory(this->tree_pointers[posn-1]);
        left_neighbour->merge_nodes(child_node);
        this->tree_pointers.erase(this->tree_pointers.begin()+posn);
        this->keys.erase(this->keys.begin()+posn-1);
        this->size--;
        this->dump();
        return;
    }

    // right redistribute
    if ((!leftRedistribute) && (posn != this->size-1))
    {
        // cout<<"Right Neighbour exists"<<endl;
        auto right_neighbour = TreeNode::tree_node_factory(this->tree_pointers[posn+1]);
        if (child_node->size + right_neighbour->size >= 2*MIN_OCCUPANCY)
        {
            // right redistribution possible!
            // cout<<"Right redistribution possible"<<endl;
            right_neighbour->redistributeData(right_neighbour, child_node, posn+1, posn);
            this->keys[posn] = child_node->max();
            rightRedistribute = true;
            this->dump();
            return;
        }
    }
    
    // If neither left redistribute nor right redistribute worked, lets try for right merge
    
    // cout<<"Right Neighbour exists. Performing Right Merge"<<endl;
    auto right_neighbour = TreeNode::tree_node_factory(this->tree_pointers[posn+1]);
    child_node->merge_nodes(right_neighbour);
    this->tree_pointers.erase(this->tree_pointers.begin()+posn+1);
    this->keys.erase(this->keys.begin()+posn);
    this->size--;
    this->dump();
    return;
    
}

//runs range query on subtree rooted at this node
void InternalNode::range(ostream &os, const Key &min_key, const Key &max_key) const {
    BLOCK_ACCESSES++;
    for (int i = 0; i < this->size - 1; i++) {
        if (min_key <= this->keys[i]) {
            auto* child_node = TreeNode::tree_node_factory(this->tree_pointers[i]);
            child_node->range(os, min_key, max_key);
            delete child_node;
            return;
        }
    }
    auto* child_node = TreeNode::tree_node_factory(this->tree_pointers[this->size - 1]);
    child_node->range(os, min_key, max_key);
    delete child_node;
}

//exports node - used for grading
void InternalNode::export_node(ostream &os) {
    TreeNode::export_node(os);
    for (int i = 0; i < this->size - 1; i++)
        os << this->keys[i] << " ";
    os << endl;
    for (int i = 0; i < this->size; i++) {
        auto child_node = TreeNode::tree_node_factory(this->tree_pointers[i]);
        child_node->export_node(os);
        delete child_node;
    }
}

//writes subtree rooted at this node as a mermaid chart
void InternalNode::chart(ostream &os) {
    string chart_node = this->tree_ptr + "[" + this->tree_ptr + BREAK;
    chart_node += "size: " + to_string(this->size) + BREAK;
    chart_node += "]";
    os << chart_node << endl;

    for (int i = 0; i < this->size; i++) {
        auto tree_node = TreeNode::tree_node_factory(this->tree_pointers[i]);
        tree_node->chart(os);
        delete tree_node;
        string link = this->tree_ptr + "-->|";

        if (i == 0)
            link += "x <= " + to_string(this->keys[i]);
        else if (i == this->size - 1) {
            link += to_string(this->keys[i - 1]) + " < x";
        } else {
            link += to_string(this->keys[i - 1]) + " < x <= " + to_string(this->keys[i]);
        }
        link += "|" + this->tree_pointers[i];
        os << link << endl;
    }
}

ostream& InternalNode::write(ostream &os) const {
    TreeNode::write(os);
    for (int i = 0; i < this->size - 1; i++) {
        if (&os == &cout)
            os << "\nP" << i + 1 << ": ";
        os << this->tree_pointers[i] << " ";
        if (&os == &cout)
            os << "\nK" << i + 1 << ": ";
        os << this->keys[i] << " ";
    }
    if (&os == &cout)
        os << "\nP" << this->size << ": ";
    os << this->tree_pointers[this->size - 1];
    return os;
}

istream& InternalNode::read(istream& is) {
    TreeNode::read(is);
    this->keys.assign(this->size - 1, DELETE_MARKER);
    this->tree_pointers.assign(this->size, NULL_PTR);
    for (int i = 0; i < this->size - 1; i++) {
        if (&is == &cin)
            cout << "P" << i + 1 << ": ";
        is >> this->tree_pointers[i];
        if (&is == &cin)
            cout << "K" << i + 1 << ": ";
        is >> this->keys[i];
    }
    if (&is == &cin)
        cout << "P" << this->size;
    is >> this->tree_pointers[this->size - 1];
    return is;
}

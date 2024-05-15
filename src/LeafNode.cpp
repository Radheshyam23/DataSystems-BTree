#include "RecordPtr.hpp"
#include "LeafNode.hpp"

LeafNode::LeafNode(const TreePtr &tree_ptr) : TreeNode(LEAF, tree_ptr) {
    this->data_pointers.clear();
    this->next_leaf_ptr = NULL_PTR;
    if(!is_null(tree_ptr))
        this->load();
}

//returns max key within this leaf
Key LeafNode::max() {
    auto it = this->data_pointers.rbegin();
    return it->first;
}

//inserts <key, record_ptr> to leaf. If overflow occurs, leaf is split
//split node is returned
TreePtr LeafNode::insert_key(const Key &key, const RecordPtr &record_ptr) 
{
    // cout<<"Entered LeafNode::insert_key, key: "<<key<<endl;

    // check if key is already there in the map data_pointers
    if (this->data_pointers.find(key) != this->data_pointers.end())
        return NULL_PTR;
    
    this->data_pointers[key] = record_ptr;
    this->size++;

    if (!this->overflows())
    {
        // cout<<"No overflow in leaf. dumping.."<<endl;
        this->dump();
        return NULL_PTR;
    }

    // cout<<"Overflow in leaf. splitting.."<<endl;
    // if overflow occurs, we need to split the node
    // we will split the node into two nodes, and return the pointer to the new node
    auto new_leaf = new LeafNode();
    auto new_leaf_ptr = new_leaf->tree_ptr;

    // Adding the new leaf node to the linked list:
    new_leaf->next_leaf_ptr = this->next_leaf_ptr;
    this->next_leaf_ptr = new_leaf_ptr;

    int mid = MIN_OCCUPANCY;

    auto it = this->data_pointers.begin();
    for(int i = 0; i < mid; i++)
        it++;
    
    // /////
    // it--;
    // cout<<"Split at: "<<it->first<<endl;
    // it++;
    // /////

    for(; it != this->data_pointers.end(); it++)
    {
        new_leaf->data_pointers[it->first] = it->second;
        new_leaf->size++;
    }

    // delete those shifted keys from the original node
    this->data_pointers.erase(this->data_pointers.find(new_leaf->data_pointers.begin()->first), this->data_pointers.end());
    
    this->size = mid;

    new_leaf->dump();
    this->dump();
    return new_leaf_ptr;    
}

void LeafNode::redistributeData(TreeNode *from, TreeNode *to, int fromIndex, int toIndex)
{
    // cout<<"Entered LeafNode::redistributeData()"<<endl;
    auto fromLeaf = dynamic_cast<LeafNode*>(from);
    auto toLeaf = dynamic_cast<LeafNode*>(to);

    if (fromIndex < toIndex)
    {
        // cout<<"LeftRedistribute\n";
        // cout<<"Shifting data from "<<fromIndex<<" to "<<toIndex<<endl;
        auto it = fromLeaf->data_pointers.rbegin();
        toLeaf->insert_key(it->first, it->second);
        fromLeaf->delete_key(it->first);
    }
    else
    {
        // cout<<"RightRedistribute\n";
        // cout<<"Shifting data from "<<fromIndex<<" to "<<toIndex<<endl;
        auto it = fromLeaf->data_pointers.begin();
        toLeaf->insert_key(it->first, it->second);
        fromLeaf->delete_key(it->first);
    }
}

// merge sibling node with itself and delete sibling node
void LeafNode::merge_nodes(TreeNode* sibling_tree_node) 
{
    // cout<<"Entered LeafNode::merge_node()"<<endl;
    LeafNode* sibling_node = dynamic_cast<LeafNode*>(sibling_tree_node);
    this->data_pointers.insert(sibling_node->data_pointers.begin(), sibling_node->data_pointers.end());
    this->size += sibling_node->size;
    this->next_leaf_ptr = sibling_node->next_leaf_ptr;
    sibling_node->delete_node();
    this->dump();
}

//key is deleted from leaf if exists
void LeafNode::delete_key(const Key &key) 
{
    // cout << "LeafNode::delete_key not implemented" << endl;
    // this->dump();
    // cout<<"Entered LeafNode::delete_key, key: "<<key<<endl;
    if (this->data_pointers.erase(key))
    {
        // cout<<"Delete Successful"<<endl;
        this->size--;
        this->dump();
    }
}

//runs range query on leaf
void LeafNode::range(ostream &os, const Key &min_key, const Key &max_key) const {
    BLOCK_ACCESSES++;
    for(const auto& data_pointer : this->data_pointers){
        if(data_pointer.first >= min_key && data_pointer.first <= max_key)
            data_pointer.second.write_data(os);
        if(data_pointer.first > max_key)
            return;
    }
    if(!is_null(this->next_leaf_ptr)){
        auto next_leaf_node = new LeafNode(this->next_leaf_ptr);
        next_leaf_node->range(os, min_key, max_key);
        delete next_leaf_node;
    }
}

//exports node - used for grading
void LeafNode::export_node(ostream &os) {
    TreeNode::export_node(os);
    for(const auto& data_pointer : this->data_pointers){
        os << data_pointer.first << " ";
    }
    os << endl;
}

//writes leaf as a mermaid chart
void LeafNode::chart(ostream &os) {
    string chart_node = this->tree_ptr + "[" + this->tree_ptr + BREAK;
    chart_node += "size: " + to_string(this->size) + BREAK;
    for(const auto& data_pointer: this->data_pointers) {
        chart_node += to_string(data_pointer.first) + " ";
    }
    chart_node += "]";
    os << chart_node << endl;
}

ostream& LeafNode::write(ostream &os) const {
    TreeNode::write(os);
    for(const auto & data_pointer : this->data_pointers){
        if(&os == &cout)
            os << "\n" << data_pointer.first << ": ";
        else
            os << "\n" << data_pointer.first << " ";
        os << data_pointer.second;
    }
    os << endl;
    os << this->next_leaf_ptr << endl;
    return os;
}

istream& LeafNode::read(istream& is){
    TreeNode::read(is);
    this->data_pointers.clear();
    for(int i = 0; i < this->size; i++){
        Key key = DELETE_MARKER;
        RecordPtr record_ptr;
        if(&is == &cin)
            cout << "K: ";
        is >> key;
        if(&is == &cin)
            cout << "P: ";
        is >> record_ptr;
        this->data_pointers.insert(pair<Key,RecordPtr>(key, record_ptr));
    }
    is >> this->next_leaf_ptr;
    return is;
}
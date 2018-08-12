// btree.cpp

#include <iostream>
#include <string>
#include "btree.h"

using namespace std;

void btree::splitChild(int i, btree* y) {
  // Create a new node which is going to store t-1 = 2 keys
  // of y
  btree* z = new btree;
  z->num_keys = 2;
  // Copy the last (t-1) keys of y to z
  for (int j = 0; j < 2; j++) {
    z->keys[j] = y->keys[j+3];
  }
    // Copy the last t children of y to z
    if (y->is_leaf == false) {
      for (int j = 0; j < 3; j++) {
        z->children[j] = y->children[j+3];
      }    
    }
    // Reduce the number of keys in y
    y->num_keys = 2;
    // Since this node is going to have a new child,
    // create space of new child
    for (int j = y->num_keys; j >= i+1; j--) {
      children[j+1] = children[j];
    } 
    // Link the new child to this node
    children[i+1] = z;
    // A key of y will move to this node. Find location of
    // new key and move all greater keys one space ahead
    for (int j = 1; j >= i; j--) {
      keys[j+1] = keys[j];
    }
    // Copy the middle key of y to this node
    keys[i] = y->keys[2];
    // Increment count of keys in this node
    y->num_keys =  y->num_keys++;
}

void btree::merge(int idx) {
  btree* child = children[idx];
  btree* sibling = children[idx+1];
  // Pulling a key from the current node and inserting it into (t-1)th
  // position of C[idx]
  child->keys[2] = keys[idx];
  // Copying the keys from C[idx+1] to C[idx] at the end
  for (int i = 0; i < sibling->num_keys; ++i) {
    child->keys[i + 3] = sibling->keys[i];
  }
  // Copying the child pointers from C[idx+1] to C[idx]
  if (!child->is_leaf) {
    for(int i=0; i<=sibling->num_keys; ++i) {
      child->children[i+3] = sibling->children[i];
    }
  }
  // Moving all keys after idx in the current node one step before -
  // to fill the gap created by moving keys[idx] to C[idx]
  for (int i = idx+1; i < num_keys; ++i) {
    keys[i-1] = keys[i];
  }
  // Moving the child pointers after (idx+1) in the current node one
  // step before
  for (int i = idx+2; i <= num_keys; ++i) {
    children[i-1] = children[i];
  }
  // Updating the key count of child and the current node
  child->num_keys += sibling->num_keys+1;
  num_keys--;
  // Freeing the memory occupied by sibling
  delete(sibling);
  return;
}

int btree::findKey(int key) {
  int idx=0;
  while (idx<num_keys && keys[idx] < key) {
    ++idx;
  }
  return idx;
}

void btree::borrowFromPrev(int idx) {
    btree* child = children[idx];
    btree* sibling = children[idx-1];
    // The last key from C[idx-1] goes up to the parent and key[idx-1]
    // from parent is inserted as the first key in C[idx]. Thus, the  loses
    // sibling one key and child gains one key
 
    // Moving all key in C[idx] one step ahead
    for (int i=child->num_keys-1; i>=0; --i) {
        child->keys[i+1] = child->keys[i];
    }
    // If C[idx] is not a leaf, move all its child pointers one step ahead
    if (!child->is_leaf) {
      for(int i=child->num_keys; i>=0; --i) {
        child->children[i+1] = child->children[i];
      }
    }
    // Setting child's first key equal to keys[idx-1] from the current node
    child->keys[0] = keys[idx-1];
    // Moving sibling's last child as C[idx]'s first child
    if(!child->is_leaf) {
        child->children[0] = sibling->children[sibling->num_keys];
    }
    // Moving the key from the sibling to the parent
    // This reduces the number of keys in the sibling
    keys[idx-1] = sibling->keys[sibling->num_keys-1];
    child->num_keys += 1;
    sibling->num_keys -= 1;
    return;
}

void btree::borrowFromNext(int idx) {
  btree* child = children[idx];
  btree* sibling = children[idx+1];
  // keys[idx] is inserted as the last key in C[idx]
  child->keys[(child->num_keys)] = keys[idx];
  // Sibling's first child is inserted as the last child
  // into C[idx]
  if (!(child->is_leaf)) {
    child->children[(child->num_keys)+1] = sibling->children[0];
  }
  //The first key from sibling is inserted into keys[idx]
  keys[idx] = sibling->keys[0];
  // Moving all keys in sibling one step behind
  for (int i=1; i<sibling->num_keys; ++i) {
    sibling->keys[i-1] = sibling->keys[i];
  }
  // Moving the child pointers one step behind
  if (!sibling->is_leaf) {
    for(int i=1; i<=sibling->num_keys; ++i) {
      sibling->children[i-1] = sibling->children[i];
    }
  }
  // Increasing and decreasing the key count of C[idx] and C[idx+1]
  // respectively
  child->num_keys += 1;
  sibling->num_keys -= 1;
  return;
}

void btree::fill(int idx) {
 // If the previous child(C[idx-1]) has more than t-1 keys, borrow a key
  // from that child
  if (idx!=0 && children[idx-1]->num_keys>=3) {
    borrowFromPrev(idx);
  }
  // If the next child(C[idx+1]) has more than t-1 keys, borrow a key
  // from that child
  else if (idx!=num_keys && children[idx+1]->num_keys>=3) {
    borrowFromNext(idx);
  }
    // Merge C[idx] with its sibling
    // If C[idx] is the last child, merge it with with its previous sibling
    // Otherwise merge it with its next sibling
  else {
    if (idx != num_keys) {
      merge(idx);
    } else {
        merge(idx-1);
      }
    }
    return;
}

// A function to get predecessor of keys[idx]
int btree::getPred(int idx) {
  // Keep moving to the right most node until we reach a leaf
  btree* cur = children[idx];
  while (!cur->is_leaf) {
    cur = cur->children[cur->num_keys];
  }
  // Return the last key of the leaf
  return cur->keys[cur->num_keys-1];
}
 
int btree::getSucc(int idx) {
  // Keep moving the left most node starting from C[idx+1] until we reach a leaf
  btree* cur = children[idx+1];
  while (!cur->is_leaf) {
    cur = cur->children[0];
  }
  // Return the first key of the leaf
  return cur->keys[0];
}

void insert(btree*& root, int key) {
     // If tree is empty
    if (root == NULL) {
      // Allocate memory for root
      root = new btree;
      root->keys[0] = key;  // Insert key
      root->num_keys = 1;  // Update number of keys in root
    } else {
      // Tree is not empty
      if(root->num_keys == 5) {
        btree* node = new btree;
        node->children[0] = root;
        node->splitChild(0,root);
        // New root has two children now. Decide which of the
        // two children is going to have new key
        int i = 0;
        if (node->keys[0] < key) {
          i++;
        }
        node->children[i]->insertNonFull(key);
        // Change root
        root = node;
      } else {  // If root is not full, call insertNonFull for root
        root->insertNonFull(key);
      }    
    }
}
 
void btree::insertNonFull(int key) {
  // Initialize index as index of rightmost element
  int i = num_keys-1;
  // If this is a leaf node
  if (is_leaf == true) {
    // The following loop does two things
    // a) Finds the location of new key to be inserted
    // b) Moves all greater keys to one place ahead
    while (i >= 0 && keys[i] > key) {
      keys[i+1] = keys[i];
      i--;
    }
    // Insert the new key at found location
    keys[i+1] = key;
    num_keys = num_keys + 1;
  } else { // if this node is not a leaf
      // Find the child which is going to have the new key
      while (i >= 0 && keys[i] > key) {
        i--;
      }
      // See if the found child is full
      if (children[i+1]->num_keys == 5) {
        // If the child is full, then split it
        splitChild(i+1, children[i+1]);
        // After split, the middle key of C[i] goes up and
        // children[i] is splitted into two.  See which of the two
        // is going to have the new key
        if (keys[i+1] < key) {
          i++;
        }  
      }
      children[i+1]->insertNonFull(key);
    }
}
 
void btree::removeFromLeaf(int idx) {
  // Move all the keys after the idx-th pos one place backward
  for (int i=idx+1; i<num_keys; ++i) {
    keys[i-1] = keys[i];
  }
  // Reduce the count of keys
  num_keys--;
  return;
}

void btree::removeFromNonLeaf(int idx) {
  int key = keys[idx];
  // If the child that precedes k (C[idx]) has atleast t keys,
  // find the predecessor 'pred' of k in the subtree rooted at
  // C[idx]. Replace k by pred. Recursively delete pred
  // in C[idx]
  if (children[idx]->num_keys >= 3) {
    int pred = getPred(idx);
    keys[idx] = pred;
    children[idx]->remove0(pred);
  }
  // If the child C[idx] has less that t keys, examine C[idx+1].
  // If C[idx+1] has atleast t keys, find the successor 'succ' of k in
  // the subtree rooted at C[idx+1]
  // Replace k by succ
  // Recursively delete succ in C[idx+1]
  else if  (children[idx+1]->num_keys >= 3) {
    int succ = getSucc(idx);
    keys[idx] = succ;
    children[idx+1]->remove0(succ);
  }
  // If both C[idx] and C[idx+1] has less that t keys,merge k and all of C[idx+1]
  // into C[idx]
  // Now C[idx] contains 2t-1 keys
  // Free C[idx+1] and recursively delete k from C[idx]
  else {
    merge(idx);
    children[idx]->remove0(key);
  }
  return;
}


void btree::remove0(int key) {
  int idx = findKey(key);
  // The key to be removed is present in this node
  if (idx < num_keys && keys[idx] == key) {
    // If the node is a leaf node - removeFromLeaf is called
    // Otherwise, removeFromNonLeaf function is called
    if (is_leaf){
      removeFromLeaf(idx);
    } else {
        removeFromNonLeaf(idx);
      }
    } else {
        // If this node is a leaf node, then the key is not present in tree
        if (is_leaf) {
          return;
        }
        // The key to be removed is present in the sub-tree rooted with this node
        // The flag indicates whether the key is present in the sub-tree rooted
        // with the last child of this node
        bool flag = ( (idx==num_keys)? true : false );
        // If the child where the key is supposed to exist has less that t keys,
        // we fill that child
        if (children[idx]->num_keys < 3){
          fill(idx);
        }
        // If the last child has been merged, it must have merged with the previous
        // child and so we recurse on the (idx-1)th child. Else, we recurse on the
        // (idx)th child which now has atleast t keys
        if (flag && idx > num_keys)
            children[idx-1]->remove0(key);
        else
            children[idx]->remove0(key);
    }
    return;
}


void remove(btree*& root, int key) {
  if(!root) { //empty tree
    return;
  }
  root->remove0(key);
  if(root->num_keys == 0) {
    btree* temp = root;
    if(root->is_leaf) {
      root = NULL;
    } else {
        root = root->children[0];
      }
      delete temp;
  }
  return;
}


btree* find(btree*& root, int key) {
  if(root == NULL) {
    return root;
  }
  for(int i=0; i< root->num_keys; i++) {
    if(root->keys[i] == key) {
      return root;
    } else if(!root->is_leaf && root->keys[i] > key) {
      return find(root->children[i], key);
      }
  }
  if(!root->is_leaf && root->keys[root->num_keys-1] < key) {
    return find(root->children[root->num_keys], key);
  }
}

//int count_nodes(btree*& root) {
//  btree::count_nodes0(root);
//}

int btree::count_nodes0(btree*& root) {
  int i;
  int count = 0;
  for (i=0; i < num_keys; i++){
    if(is_leaf == false){
      children[i]-> count_nodes0(root);
      count++;
    }
  }
  return count;
}
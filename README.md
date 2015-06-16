## Edge-Weighted Personalized PageRank

This release contains implementations from paper
"Edge-Weighted Personalized PageRank: Breaking A Decade-Old Performance Barrier" (KDD 2015).

Here we use [ObjectRank](http://www.vldb.org/conf/2004/RS15P2.PDF) on the [DBLP dataset](http://cs.aminer.org/citation) as an illustration.


### Requirements
* numpy (>= 1.6.2)
* scipy (>= 0.10.1)

### Usage
Download the preprocessed data from [here](https://drive.google.com/folderview?id=0B91zl_48PQe1fm9kVjVna0NReFVDTEVCZ1p1RnB2WW9tTml2WWJSTWktSUF0NlR4T0h1NjA&usp=sharing), put it under this folder and decompress it:
```
tar xvf data.tar.gz
```

Run the following commands to execute the experiments for query answering and learning to rank with model reduction method:
```
script/answerquery.sh
script/learnrank.sh
```

After the execution, the experiemental results will be generated in the ```result``` folder.


### Comming soon
* Code for preprocessing


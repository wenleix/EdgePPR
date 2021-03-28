## Edge-Weighted Personalized PageRank

This release contains implementations from paper
"Edge-Weighted Personalized PageRank: Breaking A Decade-Old Performance Barrier" (KDD 2015).

Here we use [ObjectRank](http://www.vldb.org/conf/2004/RS15P2.PDF) on the [DBLP dataset](http://cs.aminer.org/citation) as an illustration.


### Requirements
* numpy (>= 1.6.2)
* scipy (>= 0.10.1)

### Usage
Download the preprocessed data `data.tar.gz` from [here](https://drive.google.com/folderview?id=0B91zl_48PQe1fm9kVjVna0NReFVDTEVCZ1p1RnB2WW9tTml2WWJSTWktSUF0NlR4T0h1NjA&usp=sharing) and decompress it:
```
tar xvf data.tar.gz
```

Run the following commands to execute the experiments for query answering and learning to rank with model reduction method:
```
script/answerquery.sh
script/learnrank.sh
```

After the execution, the experiemental results will be generated in the ```result``` folder.


### Preprocessing

1. Download the processed DBLP graph file `dblp_obj.txt.zip` from [here](https://drive.google.com/folderview?id=0B91zl_48PQe1fm9kVjVna0NReFVDTEVCZ1p1RnB2WW9tTml2WWJSTWktSUF0NlR4T0h1NjA&usp=sharing), put the unzipped `dblp_obj.txt` under `data/` directory.

2. Compile the code 
```
make -j4
```

3. Run the following command to generate sampled Personalized PageRank vectors:
```
bin/ParamPPR data/dblp_obj.txt 2191288 data/sample-params.txt data/sample-vecs/value bin 12
```
The last parameter `12` is number of threads for computing. You may adjust it based on the number of cores in your machine.

4. TODO: Generate reduced space, etc. 


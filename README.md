# rdv_rs_bag_to_video

## realsense-viewer로 저장한 bag파일을 image파일 혹은 mp4 video파일로 변환


1. Get source code
<pre><code>$ git clone https://github.com/rendezvue/rdv_rs_bag_to_video.git<br>
$ cd rdv_rs_bag_to_video</code></pre>

2. cmake
<pre><code>rdv_rs_bag_to_video$ mkdir build<br>
rdv_rs_bag_to_video$ cd build<br>
rdv_rs_bag_to_video/build$ cmake ..</code></pre>

3. compile
<pre><code>rdv_rs_bag_to_video/build$ make</code></pre>

4. run
<pre><code>rdv_rs_bag_to_video/build$ ./rdv_rs_bag_to_video --bag ../bag/test.bag --v</code></pre>
* option : --bag "rs bag파일 경로" : load할 bag파일의 경로
* option : --i : image file 저장
* option : --v : video file 저장

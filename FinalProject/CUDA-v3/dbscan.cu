/*
1.���������ĺ�������
2.��1�Ż����޸���GPU���ظ�����ռ�Ĳ���
3.��region query���л�������������Ҫ����Ľ��ȫ�����м������֮��������鼴�ɵõ������

*/


#include "dbscan.h"
#include <math.h>
#include <queue>
#include<iostream>
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
using namespace std;

/*
// calculate eculidean distance of two 2-D points
double euclidean_distance(Point a, Point b)
{
double x = a.x-b.x;
double y = a.y-b.y;
return sqrt(x*x+y*y);
}

// get neighborhood of point p and add it to neighborhood queue

int region_query( vector<Point> &dataset, int p, queue<int> &neighborhood, double eps)
{
	//int count = 0;
	for (int i = 0; i < dataset.size(); i++) {
		//cout << "regin_query" << count++ << endl;
		if(i!=p){
			int dist = euclidean_distance(dataset[p],dataset[i]);
			if ( dist< eps) {
				neighborhood.push(i);
			}
		}
	}
	return (int)neighborhood.size();
}
*/

unsigned int total = 0;
double* dev_nodeX;
double* dev_nodeY;
int* dev_result;
int* dev_p;
double* dev_eps;
int *result;
int datasize;
int** total_query_result;
//vector<int> *final_result;
//int queuesize;


/*
__device__ int cal(double *dev_nodeX, double *dev_nodeY, int* dev_p, int* dev_i,double* dev_eps)
{
	int i = *dev_i;
	if (i != *dev_p)
	{
		//int dist=euclidean_distance(dev_nodeX[i],dev_nodeY[i],dev_nodeX[*dev_p],dev_nodeY[*dev_p]);
		double x = dev_nodeX[i] - dev_nodeX[*dev_p];
		double y = dev_nodeY[i] - dev_nodeY[*dev_p];
		int dist = sqrt(x*x + y*y);
		//if (dist<*dev_eps) printf(" #%d (%.3f, %.3f) -> #%d(%.3f,%.3f) dist is %d\n",i, dev_nodeX[i], dev_nodeY[i], *dev_p, dev_nodeX[*dev_p], dev_nodeY[*dev_p], dist);
		if (dist<*dev_eps)
		{
			return 1;
		}
	}
	return 0;
}
*/
__global__ void region_query_kernal(double *dev_nodeX, double *dev_nodeY,int* dev_query_size, int* dev_query_target, int *dev_pointer, double* dev_eps)
{
	int size = *dev_query_size;
	int tid = threadIdx.x;
	int bid = blockIdx.x;
	
	int index=bid*blockDim.x+tid;
	
	//printf("%d ah\n",index);
	
	
	int i=index/size;//��ǰ������ǵڼ��У��б��
	
	int j=index%size;//��ǰ������ǵڼ���Ԫ�أ��б��
		
	int target = dev_query_target[i];//��ǰ���������һ��Ŀ��Ԫ��
	
	//int value=cal(dev_nodeX,dev_nodeY,&target,&j,dev_eps);//���㵱ǰ�����Ԫ����Ŀ��Ԫ�صľ��룬�������Ҫ�󷵻�1�����򷵻�0
	
	int value=0;
	
	if (target != j)
	{
		//int dist=euclidean_distance(dev_nodeX[i],dev_nodeY[i],dev_nodeX[*dev_p],dev_nodeY[*dev_p]);
		double x = dev_nodeX[j] - dev_nodeX[target];
		double y = dev_nodeY[j] - dev_nodeY[target];
		int dist = sqrt(x*x + y*y);
		//if (dist<*dev_eps) printf(" #%d (%.3f, %.3f) -> #%d(%.3f,%.3f) dist is %d\n",i, dev_nodeX[i], dev_nodeY[i], *dev_p, dev_nodeX[*dev_p], dev_nodeY[*dev_p], dist);
		if (dist<*dev_eps)
		{
			value=1;
		}
	}
	
	if(value){//�����ǰ�����Ԫ�ط���Ҫ�����-9999�������1
		//printf("???\n");
		dev_pointer[index]=-9999;
	}
	else{
		dev_pointer[index]=1;
	}
}

void pral_query(int*query_target,int**total_query_result, double eps)
{
	
	//int *query_target;//�洢Ҫ�������Ԫ��
	//int **query_result;//���
	int *pointer;
	//cudaError_t cudaStatus;
	
	int *dev_query_target;
	//int **dev_query_result;
	int *dev_pointer;
	int *dev_query_size;
	
	
	//cudaMalloc((void***)&dev_query_result, datasize*sizeof(int*));
	cudaMalloc((void**)&dev_pointer, datasize*datasize*sizeof(int));
	//query_result=(int**)malloc(datasize*sizeof(int*));
	pointer=(int*)malloc(datasize*datasize*sizeof(int));
	/*
	for(int i=0;i<datasize;i++)
	{
		query_result[i]=dev_pointer+i*datasize;
	}
	*/
	//cudaMemcpy(dev_query_result, query_result, datasize*sizeof(int*), cudaMemcpyHostToDevice);
	
	cudaMalloc((void**)&dev_query_target, datasize*sizeof(int));
	cudaMalloc((void**)&dev_query_size, sizeof(int));
	
	
	cudaMemcpy(dev_query_size, &datasize, sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy(dev_query_target, query_target, datasize*sizeof(int), cudaMemcpyHostToDevice);
	
	//printf("step into kernal\n");
	region_query_kernal<<<(datasize*datasize+511)/512,512>>>(dev_nodeX, dev_nodeY,dev_query_size, dev_query_target, dev_pointer, dev_eps);
	//printf("step out of kernal\n");
	//cudaError_t error = cudaGetLastError();
	//printf("CUDA error: %s\n", cudaGetErrorString(error));
	
	cudaThreadSynchronize();

	cudaMemcpy(pointer, dev_pointer, datasize*datasize*sizeof(int), cudaMemcpyDeviceToHost);
	
	for(int p=0;p<datasize*datasize;p++)
	{
		int i=p/datasize;
		int j=p%datasize;
		total_query_result[i][j]=pointer[p];
	}
	
	//free(query_result);
	free(pointer);
	cudaFree(dev_query_target);
	//cudaFree(dev_query_result);
	cudaFree(dev_query_size);cudaFree(dev_pointer);

		
}

// expand cluster formed by p, which works in a way of bfs.
bool expand_cluster(vector<Point> &dataset, int p, int c, double eps, int min_pts) {
	queue<int> neighbor_pts;
	dataset[p].lable = c;


	for(int i=0;i<datasize;i++)
	{
		if(total_query_result[p][i]==-9999)
			neighbor_pts.push(i);
	}

	
	while (!neighbor_pts.empty()) {

		int neighbor = neighbor_pts.front();
		queue<int> neighbor_pts1;

		//printf("neighbor is %d\n",neighbor);
		//printf("step into query2\n");
		//region_query(dataset, neighbor, neighbor_pts1, eps);
		//printf("step out of query2\n");
		for(int i=0;i<datasize;i++)
		{
			if(total_query_result[neighbor][i]==-9999)
				neighbor_pts1.push(i);
		}

		if (neighbor_pts1.size() >= min_pts - 1)
		{
			while (!neighbor_pts1.empty()) {

				int pt = neighbor_pts1.front();
				if (dataset[pt].lable == -1) {
					neighbor_pts.push(pt);
				}
				neighbor_pts1.pop();
			}
		}
		dataset[neighbor].lable = c;
		neighbor_pts.pop();

	}
	
	return  true;
}

void allocate_data_init(vector<Point> &dataset, double eps){
// device memory allocate
	int size = dataset.size();
	datasize=size;
	double *nodeX = (double*)malloc(size * sizeof(double));
	double *nodeY = (double*)malloc(size * sizeof(double));

	for (int i = 0; i<size; i++)
	{
		nodeX[i] = dataset[i].x;
		nodeY[i] = dataset[i].y;
	}
	
	cudaMalloc((void**)&dev_nodeX, size * sizeof(double));
	cudaMalloc((void**)&dev_nodeY, size * sizeof(double));
	cudaMalloc((void**)&dev_eps, sizeof(double));
	cudaMemcpy(dev_nodeX, nodeX, size * sizeof(double), cudaMemcpyHostToDevice);
	cudaMemcpy(dev_nodeY, nodeY, size * sizeof(double), cudaMemcpyHostToDevice);
	cudaMemcpy(dev_eps, &eps, sizeof(double), cudaMemcpyHostToDevice);

	//cudaMalloc((void**)&dev_result, size * sizeof(int));
	//cudaMalloc((void**)&dev_p, sizeof(int));

	free(nodeX); free(nodeY);
//  host memory allocate
	//result = (int*)malloc(size * sizeof(int));
	
	//final_result=(vector<int>*)malloc(size*queuesize);
	
}

void allocate_data_free() {
// device memory free
	cudaFree(dev_nodeX); cudaFree(dev_nodeY); 
	//cudaFree(dev_result);
	//cudaFree(dev_p); 
	cudaFree(dev_eps);
// host memory free
	//free(result);
	
	//free(final_result);
}

// doing dbscan, given radius and minimum number of neigborhoods.
int dbscan(vector<Point> &dataset, double eps, int min_pts)
{
	int c = 0;  // cluster lable
	//int count = 0;
	int p;
	
	int size=dataset.size();
	
	int* query_target=(int*) malloc(size*sizeof(int));
	
	for(int i=0;i<size;i++) query_target[i]=i;
	
	//queuesize=sizeof(query_target);
	
	allocate_data_init(dataset, eps);//�����Ѿ���dev_nodeX,dev_nodeY,dev_eps������GPU��
	
	
	//����ÿһ�ε���pral_query����������ѽ���������������Ȼ�������Ϊ��������
	//��������е�ÿһ�� ��Ӧtarget�е�һ��Ԫ�ص��ھ�
	total_query_result = (int**)malloc(size*sizeof(int*));
	for(int i=0;i<size;i++)
	{
		total_query_result[i]=(int*)malloc(size*sizeof(int));
	}
	//�������ÿ��Ԫ�ص��ھ�
	//printf("start to pral\n");
	clock_t start, finish;
	double duration;
	start = clock();
	
	pral_query(query_target,total_query_result, eps);//query_target�е�Ԫ�ر��߳�
	
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	cout << "pral: "<< duration << "s" << endl;
	//printf("finish to pral\n");
	free(query_target);
	
	
	for (p = 0; p<size; p++) {
		queue<int> neighborhood;
		//printf("%d query start\n", count++);
		for(int i=0;i<size;i++)
		{
			if(total_query_result[p][i]==-9999) neighborhood.push(i);
			//printf("%d done\n",i);
		}
		//printf("%d query end\n", count++);
		//region_query(dataset, p, neighborhood, eps);//�ҵ�p�����ڽڵ㣬����֮��ľ���С��eps�������ڽڵ��ŵ�������
													//printf("%d query end\n",count);
		if (neighborhood.size() + 1 < min_pts) {//���p����ڵ���������ŵĴ�С С����С��Ҫ�󣬽�����Ϊ0
												// mark as noise
			//printf("miaomiaomiao?");
			dataset[p].lable = 0;
		}
		else
		{

			if (dataset[p].lable == -1) {//�������pû�б����࣬�������ڽڵ���չ
				c++;
				//printf("step into cluster\n");
				expand_cluster(dataset, p, c, eps, min_pts);
				//printf("step out of cluster\n");
			}
		}
	}
	free(total_query_result);
	allocate_data_free();
	return c;
}
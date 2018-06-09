
//a node in the bplustree is represented as an offset
//the real node struct:xx_node
#include<cstdio>
#include<cstdlib>

#define MAX_KEY 5

typedef int Key;
typedef unsigned int Pointer;
typedef unsigned int Offset;//in the file

struct Key_Pointer
{
	Key key;
	Pointer pointer;
};

class Node
{
public:
	Key key[MAX_KEY];
	Pointer pointer[MAX_KEY + 1];
	int key_total;
	bool is_leaf;
	Node() {
		for (int i = 0; i<MAX_KEY; ++i)
		{
			key[i] = -1;
			pointer[i] = -1;// ??MAX_KEY is the pointer or xx.key_total
		}
		pointer[MAX_KEY] = -1;
		key_total = 0;
		is_leaf = false;
	}
	void copy(Node& dest) {
		for (int i = 0; i<MAX_KEY; ++i)
		{
			dest.key[i] = key[i];
			dest.pointer[i] = pointer[i];// ??MAX_KEY is the pointer or xx.key_total
		}
		dest.pointer[MAX_KEY] = pointer[MAX_KEY];
		dest.key_total = key_total;
		dest.is_leaf = is_leaf;
	}
	void print() {
		for (int i = 0; i<MAX_KEY; ++i)
		{
			printf("key[%d]=%d\t", i, key[i]);
			printf("pointer[%d]=%u\t", i, pointer[i]);
			printf("\n");
		}
		printf("pointer[%d]=%u\n", MAX_KEY, pointer[MAX_KEY]);

		printf("key_total=%d\n", key_total);
		printf("is_leaf=%d\n", is_leaf);
	}
};

void print_index(const char filename[], int size1)
{
	FILE* fp;
	fp = fopen("index", "rb");
	fseek(fp, 0, SEEK_END);
	int total_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	for (int pos = 0; pos<total_size; pos += size1)
	{
		fseek(fp, pos, SEEK_SET);
		Node n;
		fread(&n, sizeof(Node), 1, fp);
		printf("offset:%d\n", pos);
		n.print();
		printf("-----------\n");

	}

}
class Bp_tree
{
	Offset root;//the offset of the root in the file; default to be 0
	FILE* bp_file;
public:
	Bp_tree(const char filename[]);
	~Bp_tree();
	void build();
	void insert(const Key_Pointer& kp);
	Pointer query(Key key, bool get_bpos = false);
	void remove(Key key);
	void toggle();

private:
	int get_final_node();
	void write_node_to_file(const Offset bp_offset, const Node& n);
	void read_node_from_file(const Offset bp_offset, Node& n);
	void insert_pos(const Key_Pointer& kp, Offset curr);
	void split(Node& parent_node, Node& child_node, const int child_num);
	Pointer query_pos(Key key, Offset curr, bool get_bpos = false);
	void remove_pos(Key key, Offset curr);
};
void Bp_tree::toggle()
{
	fclose(bp_file);
	bp_file = fopen("index", "rb+");
}
Bp_tree::Bp_tree(const char filename[])
{
	bp_file = fopen(filename, "rb+");
	root = 0;
}
Bp_tree::~Bp_tree()
{
	fclose(bp_file);
}
void Bp_tree::build()
{
	int start = get_final_node();
	if (start != 0)
	{
		printf("Building from an existing file...\n");
		return;
	}
	printf("Building from an empty file...\n");
	Node n;
	n.pointer[MAX_KEY] = 0;
	n.key_total = 0;
	n.is_leaf = true;
	write_node_to_file(start, n);
}
int Bp_tree::get_final_node()
{
	fseek(bp_file, 0, SEEK_END);
	return ftell(bp_file);
}

void Bp_tree::write_node_to_file(const Offset bp_offset, const Node& n)
{
	fseek(bp_file, bp_offset, SEEK_SET);
	fwrite(&n, sizeof(Node), 1, bp_file);
}
void Bp_tree::read_node_from_file(const Offset bp_offset, Node& n)
{
	fseek(bp_file, bp_offset, SEEK_SET);
	fread(&n, sizeof(Node), 1, bp_file);
}
void Bp_tree::insert(const Key_Pointer& kp)
{
	//printf("Inserting a key-pointer pair...\n");
	Node root_node;
	read_node_from_file(root, root_node);
	if (root_node.key_total == MAX_KEY)
	{
		//printf("Spliting the root...\n");
		int last = get_final_node();
		Node new_root_node;
		new_root_node.is_leaf = false;
		new_root_node.key_total = 0;
		new_root_node.key[0] = root_node.key[root_node.key_total - 1];
		new_root_node.pointer[0] = root;//I suppose the final pointer to be null


		write_node_to_file(last, new_root_node);

		split(new_root_node, root_node, 0);
		Node tmp_node;
		new_root_node.copy(tmp_node);

		tmp_node.pointer[0] = last;
		write_node_to_file(last, root_node);
		write_node_to_file(0, tmp_node);

		root = 0;
	}
	insert_pos(kp, root);
}
void Bp_tree::insert_pos(const Key_Pointer& kp, Offset curr)
{
	//printf("\tInserting a key-pointer to a pos...\n");
	//fseek(bp_file,curr,SEEK_SET);
	Node curr_node;
	read_node_from_file(curr, curr_node);
	int i;
	for (i = 0; i<curr_node.key_total&&curr_node.key[i]<kp.key; ++i);
	//!! i<curr_node.key_total����Ҫ�ģ�����Ȼ���ܸպþͺ���һ��key�����ǵ�key���
	if (i<curr_node.key_total&&curr_node.key[i] == kp.key)//already exists
		return;


	if (curr_node.is_leaf == false)//not a leaf
	{
		Node child_node;
		read_node_from_file(curr_node.pointer[i], child_node);
		if (child_node.key_total == MAX_KEY)
		{
			if (kp.key == 8)
			{
				int a = 1;
			}
			//split(curr,i); i is not the offset of the curr
			if (i == 1)
			{
				i = 1;
			}
			split(curr_node, child_node, i);

			write_node_to_file(curr, curr_node);

			write_node_to_file(curr_node.pointer[i], child_node);


		}

		if (curr_node.key[i] >= kp.key || i == curr_node.key_total)
		{
			insert_pos(kp, curr_node.pointer[i]);
		}
		else
		{
			insert_pos(kp, curr_node.pointer[i + 1]);
		}
	}
	else
	{
		//int i;
		//for(i=0;i<curr_node.key_total&&curr_node.key[i]<kp.key;++i);
		//if(curr_node.key[i]==kp.key)
		//  return;
		int inserted_pos = i;
		for (int j = curr_node.key_total; j > inserted_pos; --j)
		{
			curr_node.key[j] = curr_node.key[j - 1];
			curr_node.pointer[j] = curr_node.pointer[j - 1];
		}

		curr_node.key[inserted_pos] = kp.key;
		curr_node.pointer[inserted_pos] = kp.pointer;
		curr_node.key_total++;
		write_node_to_file(curr, curr_node);
	}

}
void Bp_tree::split(Node& parent_node, Node& child_node, const int child_num)
{
	//printf("Splitting...\n");

	int half = MAX_KEY >> 1;
	parent_node.pointer[parent_node.key_total + 1] = parent_node.pointer[parent_node.key_total];
	for (int j = parent_node.key_total; j>child_num; --j)
	{
		parent_node.key[j] = parent_node.key[j - 1];
		parent_node.pointer[j] = parent_node.pointer[j - 1];
	}
	parent_node.key[child_num] = child_node.key[half];
	parent_node.key_total++;

	int right = get_final_node();
	Node right_node;
	right_node.key_total = MAX_KEY - half - 1;
	for (int i = half + 1; i<MAX_KEY; ++i)
	{
		right_node.key[i - half - 1] = child_node.key[i];
		right_node.pointer[i - half - 1] = child_node.pointer[i];
	}
	right_node.is_leaf = child_node.is_leaf;

	//right_node.key[right_node.key_total] = child_node.key[MAX_KEY];
	right_node.pointer[right_node.key_total] = child_node.pointer[MAX_KEY];


	parent_node.pointer[child_num + 1] = right;


	child_node.key_total = half;
	if (child_node.is_leaf == true)
	{
		child_node.key_total++;

		right_node.pointer[MAX_KEY] = child_node.pointer[MAX_KEY];
		child_node.pointer[MAX_KEY] = right;
	}

	write_node_to_file(right, right_node);
}

// if not find, return -1
//if get_bpos is true,then we find the pos of leaf in the bp_file(0,52,..)
Pointer Bp_tree::query(Key key, bool get_bpos)
{
	return  query_pos(key, root, get_bpos);
}

Pointer Bp_tree::query_pos(Key key, Offset curr, bool get_bpos)
{
	Node curr_node;
	read_node_from_file(curr, curr_node);


	//
	//�п�����ɾ����û�д����һ��״̬������˵���ڵ����滹û�и�ֵ����������Ӧ���ٴ�ɾ��ʱ�ſ��Եģ�
	if (curr_node.key_total == 0 && curr_node.is_leaf == false)
	{
		return query_pos(key, curr_node.pointer[0], get_bpos);
	}

	if (!curr_node.is_leaf)
	{
		int i;
		for (i = 0; i < curr_node.key_total&&curr_node.key[i] < key; ++i);
		return query_pos(key, curr_node.pointer[i], get_bpos);
	}
	else
	{
		int i;
		for (i = 0; i < curr_node.key_total&&curr_node.key[i] < key; ++i);

		//!! i < curr_node.key_total
		if (i<curr_node.key_total&&curr_node.key[i] == key)
			return get_bpos ? curr : curr_node.pointer[i];
		else
			return -1;
	}
}

void Bp_tree::remove(Key key)
{
	remove_pos(key, root);

}

void Bp_tree::remove_pos(Key key, Offset curr)
{
	int i, j;

	Node curr_node;
	read_node_from_file(curr, curr_node);

	//
	//���ӽڵ�ֻ��һ����ʱ�򣬾�Ҫ�����ߡ�
	//���Ҵ�ʱ�ýڵ�ֻ�����Ǹ��ڵ㡣����ýڵ����ڽڵ㣬����key_totalΪ0��������Ϊ1֮ǰ��������ֵܣ��ͺ��ֵܺϲ��ˡ�
	//���û���ֵܣ�������Ӧ�����ڸýڵ�ĸ��ڵ�ʹ����ˣ���ͬ�������������������ֻ�����Ǹýڵ�Ϊ���ڵ�
	if (curr_node.key_total == 0)
	{
		Node child_node;
		read_node_from_file(curr_node.pointer[0], child_node);
		child_node.copy(curr_node);
		write_node_to_file(curr, curr_node);
		remove_pos(key, curr);
		return;

	}


	for (i = 0; i < curr_node.key_total && key > curr_node.key[i]; i++);

	if (i < curr_node.key_total && curr_node.key[i] == key)  //�ڵ�ǰ�ڵ��ҵ��ؼ���  
	{

		if (!curr_node.is_leaf)     //���ڽڵ��ҵ��ؼ���  
		{
			Node child_node;
			read_node_from_file(curr_node.pointer[i], child_node);

			if (child_node.is_leaf)     //���������Ҷ�ڵ�  
			{
				if (child_node.key_total > MAX_KEY / 2)      //���A  
				{//ֻҪ�Ѻ��ӵ����һ����ɾ���Ϳ�����
					curr_node.key[i] = child_node.key[child_node.key_total - 2];
					child_node.key_total--;

					write_node_to_file(curr, curr_node);
					write_node_to_file(curr_node.pointer[i], child_node);

					return;
				}
				else    //�����ӽڵ�Ĺؼ�������������  
				{
					if (i > 0)      //�����ֵܽڵ�  
					{
						Node lbchild_node;
						read_node_from_file(curr_node.pointer[i - 1], lbchild_node);

						if (lbchild_node.key_total > MAX_KEY / 2)        //���B  
						{
							for (j = child_node.key_total; j > 0; j--)//child_node��������һ����λ���ճ�0λ��
							{
								child_node.key[j] = child_node.key[j - 1];
								child_node.pointer[j] = child_node.pointer[j - 1];
							}

							child_node.key[0] = curr_node.key[i - 1];//child_node��0λ�÷�����ֵܵ����һ��key-pointer
							child_node.pointer[0] = lbchild_node.pointer[lbchild_node.key_total - 1];

							child_node.key_total++;//?? �Ҹо����ﲻ�ñ��

							lbchild_node.key_total--;

							//���¸��ڵ�
							curr_node.key[i - 1] = lbchild_node.key[lbchild_node.key_total - 1];//������µ����ֵ
							curr_node.key[i] = child_node.key[child_node.key_total - 2];//�ұ�Ҳ���µ����ֵ

							write_node_to_file(curr, curr_node);
							write_node_to_file(curr_node.pointer[i - 1], lbchild_node);
							write_node_to_file(curr_node.pointer[i], child_node);

						}
						else    //���C  ����ֵܲ�����
						{
							//lbchild starts
							for (j = 0; j < child_node.key_total; j++)//�ұ߽ڵ��key_pointer�ŵ���߽ڵ�����
							{
								lbchild_node.key[lbchild_node.key_total + j] = child_node.key[j];
								lbchild_node.pointer[lbchild_node.key_total + j] = child_node.pointer[j];
							}
							lbchild_node.key_total += child_node.key_total;

							lbchild_node.pointer[MAX_KEY] = child_node.pointer[MAX_KEY];
							//lbchild ends
							//�ͷ�child�ڵ�ռ�õĿռ�x.pointer[i]  

							// p0 k0,p1 k1,p2 k2,p3
							// suppose i:1
							// => p0 k1,p2 k2,p3	(k1 is deleted)
							for (j = i - 1; j < curr_node.key_total - 1; j++)
							{
								curr_node.key[j] = curr_node.key[j + 1];
								curr_node.pointer[j + 1] = curr_node.pointer[j + 2];
							}
							curr_node.key_total--;

							//k1 is deleted, so use the new k1.��ʱ�����key_total��û��-1,so "-2" here
							//�����Ǵ���Ҷ�ڵ��ʱ�򣬲Ż��ȥ���1
							curr_node.key[i - 1] = lbchild_node.key[lbchild_node.key_total - 2];

							write_node_to_file(curr, curr_node);
							write_node_to_file(curr_node.pointer[i - 1], lbchild_node);

							i--;

						}


					}
					else      //ֻ�����ֵܽڵ�  
					{
						Node rbchild_node;
						read_node_from_file(curr_node.pointer[i + 1], rbchild_node);

						if (rbchild_node.key_total > MAX_KEY / 2)        //���D  �ұߵ��ֵܹ���������һ���ڵ�
						{
							//curr_node starts
							curr_node.key[i] = rbchild_node.key[0];
							//curr_node ends

							//child_node starts
							child_node.key[child_node.key_total] = rbchild_node.key[0];
							child_node.pointer[child_node.key_total] = rbchild_node.pointer[0];
							child_node.key_total++;
							//child_node starts

							//rbchild_node starts
							for (j = 0; j < rbchild_node.key_total - 1; j++)
							{
								rbchild_node.key[j] = rbchild_node.key[j + 1];
								rbchild_node.pointer[j] = rbchild_node.pointer[j + 1];
							}
							rbchild_node.key_total--;
							//rbchild_node ends

							write_node_to_file(curr, curr_node);
							write_node_to_file(curr_node.pointer[i], child_node);
							write_node_to_file(curr_node.pointer[i + 1], rbchild_node);

						}
						else    //���E  �ұߵ��ֵܲ�����ֻ�ܺϲ���
						{
							//child_node starts
							for (j = 0; j < rbchild_node.key_total; j++)
							{
								child_node.key[child_node.key_total + j] = rbchild_node.key[j];
								child_node.pointer[child_node.key_total + j] = rbchild_node.pointer[j];
							}
							child_node.key_total += rbchild_node.key_total;

							child_node.pointer[MAX_KEY] = rbchild_node.pointer[MAX_KEY];
							//child_node ends

							//�ͷ�rbchildռ�õĿռ�x.Pointer[i+1]  

							//curr_node starts
							for (j = i; j < curr_node.key_total - 1; j++)
							{
								curr_node.key[j] = curr_node.key[j + 1];
								curr_node.pointer[j + 1] = curr_node.pointer[j + 2];
							}
							curr_node.key_total--;
							//curr_node ends


							write_node_to_file(curr, curr_node);
							write_node_to_file(curr_node.pointer[i], child_node);

						}

					}

				}

			}
			else      //���F  �������ڽڵ�
			{

				//�ҵ�key��B+��Ҷ�ڵ�����ֵܹؼ���,������ؼ���ȡ��key��λ��  

				/*TRecord trecord;
				trecord.key = record.key;
				SearchResult result;
				Search_BPlus_Tree(trecord, result);*/
				Pointer result = query(key, true);


				Node last_node;

				read_node_from_file(result, last_node);

				curr_node.key[i] = last_node.key[last_node.key_total - 2];//��Ҷ�ڵ�ĵ�����һ��key_pointer�ӵ��ҵ�λ��(���һ��ɾ����)

				write_node_to_file(curr, curr_node);


				if (child_node.key_total > MAX_KEY / 2)        //���H  �ӽڵ������
				{

				}
				else          //�����ӽڵ�Ĺؼ�������������,���ֵܽڵ��ĳһ���ؼ�����������  
				{
					if (i > 0)  //x.key[i]�����ֵ�  
					{
						Node lbchild_node;
						read_node_from_file(curr_node.pointer[i - 1], lbchild_node);

						if (lbchild_node.key_total > MAX_KEY / 2)       //���I  ��ߵ��ֵܹ���������һ��
						{
							//child_node starts
							for (j = child_node.key_total; j > 0; j--)
							{
								child_node.key[j] = child_node.key[j - 1];
								child_node.pointer[j + 1] = child_node.pointer[j];
							}
							child_node.pointer[1] = child_node.pointer[0];
							child_node.key[0] = curr_node.key[i - 1];
							child_node.pointer[0] = lbchild_node.pointer[lbchild_node.key_total];

							child_node.key_total++;
							//child_node ends

							//curr_node starts
							curr_node.key[i - 1] = lbchild_node.key[lbchild_node.key_total - 1];
							//curr_node ends

							//lbchild_node starts
							lbchild_node.key_total--;
							//lbchild_node ends

							write_node_to_file(curr, curr_node);
							write_node_to_file(curr_node.pointer[i - 1], lbchild_node);
							write_node_to_file(curr_node.pointer[i], child_node);
						}
						else        //���J  ��ߵ��ֵܲ�����Ҫ�ϲ���
						{
							//lbchild_node starts
							//??����һ�ж����˰� ����forѭ�����Ѿ��ָ�ֵ��
							//!!���ã���Ϊ����key_total++��						
							lbchild_node.key[lbchild_node.key_total] = curr_node.key[i - 1];   //�����ӽڵ㸴�Ƶ������ֵܵ�ĩβ  
							lbchild_node.key_total++;

							for (j = 0; j < child_node.key_total; j++)      //��child�ڵ㿽����lbchild�ڵ��ĩβ,  
							{
								lbchild_node.key[lbchild_node.key_total + j] = child_node.key[j];
								lbchild_node.pointer[lbchild_node.key_total + j] = child_node.pointer[j];//??lbchild��pointer[key_total]�ǲ��Ǳ�������
							}
							lbchild_node.pointer[lbchild_node.key_total + j] = child_node.pointer[j];
							lbchild_node.key_total += child_node.key_total;        //�Ѿ���child������lbchild�ڵ�  
																				   //lbchild_node ends


																				   //�ͷ�child�ڵ�Ĵ洢�ռ�,curr_node.Pointer[i]  


																				   //curr_node starts
																				   //���ҵ��ؼ��ֵĺ���child��ؼ������ֵܵĺ���lbchild�ϲ���,���ùؼ���ǰ��,ʹ��ǰ�ڵ�Ĺؼ��ּ���һ��  
							for (j = i - 1; j < curr_node.key_total - 1; j++)
							{
								curr_node.key[j] = curr_node.key[j + 1];
								curr_node.pointer[j + 1] = curr_node.pointer[j + 2];
							}
							curr_node.key_total--;
							//curr_node ends


							write_node_to_file(curr, curr_node);
							write_node_to_file(curr_node.pointer[i - 1], lbchild_node);

							i--;

						}

					}
					else        //����x.key[i]ֻ�����ֵ�  
					{
						Node rbchild_node;
						read_node_from_file(curr_node.pointer[i + 1], rbchild_node);

						if (rbchild_node.key_total > MAX_KEY / 2)     //���K  ���ֵܹ���������һ��
						{

							//child_node starts ��һ��Ҷ�ڵ�������������Լ���key
							child_node.key[child_node.key_total] = curr_node.key[i];
							child_node.key_total++;
							child_node.pointer[child_node.key_total] = rbchild_node.pointer[0];
							//child_node starts


							//curr_node starts 
							curr_node.key[i] = rbchild_node.key[0];
							//curr_node ends

							//rbchild_node starts������һ��
							for (j = 0; j < rbchild_node.key_total - 1; j++)
							{
								rbchild_node.key[j] = rbchild_node.key[j + 1];
								rbchild_node.pointer[j] = rbchild_node.pointer[j + 1];
							}
							rbchild_node.pointer[j] = rbchild_node.pointer[j + 1];
							rbchild_node.key_total--;
							//rbchild_node ends

							write_node_to_file(curr, curr_node);
							write_node_to_file(curr_node.pointer[i], child_node);
							write_node_to_file(curr_node.pointer[i + 1], rbchild_node);

						}
						else        //���L  ���ֵܲ������ϲ�
						{
							//child_node starts
							//??�����Ǹ�Ӧ�ô�j=1��ʼ����total�ΰ�
							//!!���ã�����Ϊkey_total++��
							child_node.key[child_node.key_total] = curr_node.key[i];
							child_node.key_total++;

							for (j = 0; j < rbchild_node.key_total; j++)     //��rbchild�ڵ�ϲ���child�ڵ��  
							{
								child_node.key[child_node.key_total + j] = rbchild_node.key[j];
								child_node.pointer[child_node.key_total + j] = rbchild_node.pointer[j];
							}
							child_node.pointer[child_node.key_total + j] = rbchild_node.pointer[j];

							child_node.key_total += rbchild_node.key_total;
							//child_node ends


							//�ͷ�rbchild�ڵ���ռ�õĿռ�,x,Pointer[i+1]  

							//curr_node starts ǰ��һ��
							for (j = i; j < curr_node.key_total - 1; j++)    //��ǰ���ؼ���֮��Ĺؼ�������һλ,ʹ�ýڵ�Ĺؼ���������һ  
							{
								curr_node.key[j] = curr_node.key[j + 1];
								curr_node.pointer[j + 1] = curr_node.pointer[j + 2];
							}
							curr_node.key_total--;
							//curr_node ends

							write_node_to_file(curr, curr_node);
							write_node_to_file(curr_node.pointer[i], child_node);

						}

					}
				}

			}

			remove_pos(key, curr_node.pointer[i]);

		}
		else  //���G  ��curr_node���ҵ���key��curr_nodeΪҶ�ڵ�
		{
			//curr_node starts
			for (j = i; j < curr_node.key_total - 1; j++)
			{
				curr_node.key[j] = curr_node.key[j + 1];
				curr_node.pointer[j] = curr_node.pointer[j + 1];
			}
			curr_node.key_total--;
			//curr_node starts

			write_node_to_file(curr, curr_node);

			return;
		}

	}
	else        //�ڵ�ǰ�ڵ�û�ҵ��ؼ���     
	{
		if (!curr_node.is_leaf)    //û�ҵ��ؼ���,��ؼ��ֱ�Ȼ��������Pointer[i]Ϊ����������  
		{
			Node child_node;
			read_node_from_file(curr_node.pointer[i], child_node);

			if (!child_node.is_leaf)      //����亢�ӽڵ����ڽڵ�  
			{
				if (child_node.key_total > MAX_KEY / 2)        //���H  
				{

				}
				else          //�����ӽڵ�Ĺؼ�������������,���ֵܽڵ��ĳһ���ؼ�����������  
				{
					if (i > 0)  //x.key[i]�����ֵ�  
					{
						Node lbchild_node;
						read_node_from_file(curr_node.pointer[i - 1], lbchild_node);

						if (lbchild_node.key_total > MAX_KEY / 2)       //���I  ��ڵ�������ˣ���һ����
						{

							//child_node starts
							//�ӽڵ�����һλ�������ֵ����������ֵܵ����һ�����ڵ�һ��λ�ã������ø��ڵ��key������һ��
							for (j = child_node.key_total; j > 0; j--)
							{
								child_node.key[j] = child_node.key[j - 1];
								child_node.pointer[j + 1] = child_node.pointer[j];
							}
							child_node.pointer[1] = child_node.pointer[0];
							child_node.key[0] = curr_node.key[i - 1];
							child_node.pointer[0] = lbchild_node.pointer[lbchild_node.key_total];

							child_node.key_total++;
							//child_node ends

							//curr_node starts
							//���ڵ�ֻҪ����һ��key[i-1]�ͺ���
							curr_node.key[i - 1] = lbchild_node.key[lbchild_node.key_total - 1];
							//curr_node ends

							//lbchild_node starts
							//��ڵ�ֻҪ�ܵ�key����-1�ͺ���
							lbchild_node.key_total--;
							//lbchild_node ends


							write_node_to_file(curr, curr_node);
							write_node_to_file(curr_node.pointer[i - 1], lbchild_node);
							write_node_to_file(curr_node.pointer[i], child_node);
						}
						else        //���J  ��ڵ㲻������һ���
						{
							//lbchild_node starts
							//���ֵܺ��ӽڵ�������һ���м��ø��ڵ��key[i-1]����,pointer[key_total]Ҳ��Ҫ
							lbchild_node.key[lbchild_node.key_total] = curr_node.key[i - 1];   //�����ӽڵ㸴�Ƶ������ֵܵ�ĩβ  
							lbchild_node.key_total++;

							for (j = 0; j < child_node.key_total; j++)      //��child�ڵ㿽����lbchild�ڵ��ĩβ,  
							{
								lbchild_node.key[lbchild_node.key_total + j] = child_node.key[j];
								lbchild_node.pointer[lbchild_node.key_total + j] = child_node.pointer[j];
							}
							lbchild_node.pointer[lbchild_node.key_total + j] = child_node.pointer[j];
							lbchild_node.key_total += child_node.key_total;        //�Ѿ���child������lbchild�ڵ�  
																				   //lbchild_node ends


																				   //�ͷ�child�ڵ�Ĵ洢�ռ�,x.Pointer[i]  


																				   //curr_node starts
																				   //���ҵ��ؼ��ֵĺ���child��ؼ������ֵܵĺ���lbchild�ϲ���,���ùؼ���ǰ��,ʹ��ǰ�ڵ�Ĺؼ��ּ���һ��  
							for (j = i - 1; j < curr_node.key_total - 1; j++)
							{
								curr_node.key[j] = curr_node.key[j + 1];
								curr_node.pointer[j + 1] = curr_node.pointer[j + 2];
							}
							curr_node.key_total--;
							//curr_node ends

							write_node_to_file(curr, curr_node);
							write_node_to_file(curr_node.pointer[i - 1], lbchild_node);

							i--;

						}

					}
					else        //����x.key[i]ֻ�����ֵ�  
					{
						Node rbchild_node;
						read_node_from_file(curr_node.pointer[i + 1], rbchild_node);

						if (rbchild_node.key_total > MAX_KEY / 2)     //���K  ���ֵܹ������ˣ���һ����
						{

							//child_node starts
							//�ӽڵ�����һ�������ֵܵĵ�һ�����м�Ҫ�и��ڵ��key[i]����
							child_node.key[child_node.key_total] = curr_node.key[i];
							child_node.key_total++;
							child_node.pointer[child_node.key_total] = rbchild_node.pointer[0];
							//child_node ends

							//curr_node starts
							//���ڵ����һ��key[i]������
							curr_node.key[i] = rbchild_node.key[0];
							//curr_node ends


							//rbchild_node starts
							for (j = 0; j < rbchild_node.key_total - 1; j++)
							{
								rbchild_node.key[j] = rbchild_node.key[j + 1];
								rbchild_node.pointer[j] = rbchild_node.pointer[j + 1];
							}
							rbchild_node.pointer[j] = rbchild_node.pointer[j + 1];
							rbchild_node.key_total--;
							//rbchild_node ends

							write_node_to_file(curr, curr_node);
							write_node_to_file(curr_node.pointer[i], child_node);
							write_node_to_file(curr_node.pointer[i + 1], rbchild_node);

						}
						else        //���L  ���ֵܲ����ã���һ���
						{
							//child_node starts
							//�ڽڵ�ϲ�ʱ��Ҫ�������ڵ��key����
							//�ӽڵ��ø��ڵ��key���ɣ������ֵ�ƴ�� pointer[key_total]����Ҫ��
							child_node.key[child_node.key_total] = curr_node.key[i];
							child_node.key_total++;

							for (j = 0; j < rbchild_node.key_total; j++)     //��rbchild�ڵ�ϲ���child�ڵ��  
							{
								child_node.key[child_node.key_total + j] = rbchild_node.key[j];
								child_node.pointer[child_node.key_total + j] = rbchild_node.pointer[j];
							}
							child_node.pointer[child_node.key_total + j] = rbchild_node.pointer[j];

							child_node.key_total += rbchild_node.key_total;
							//child_node ends

							//�ͷ�rbchild�ڵ���ռ�õĿռ�,x,Pointer[i+1]  

							//curr_node starts
							//���ڵ�����һ��
							for (j = i; j < curr_node.key_total - 1; j++)    //��ǰ���ؼ���֮��Ĺؼ�������һλ,ʹ�ýڵ�Ĺؼ���������һ  
							{
								curr_node.key[j] = curr_node.key[j + 1];
								curr_node.pointer[j + 1] = curr_node.pointer[j + 2];
							}
							curr_node.key_total--;
							//curr_node ends

							write_node_to_file(curr, curr_node);
							write_node_to_file(curr_node.pointer[i], child_node);

						}

					}
				}
			}
			else  //�����亢�ӽڵ�����ڵ�(Ҷ�ڵ�)
			{
				if (child_node.key_total > MAX_KEY / 2)  //���M  �ӽڵ�״̬�ܺã����ô���
				{

				}
				else        //�����ӽڵ㲻������  
				{
					if (i > 0) //�����ֵ�  
					{
						Node lbchild_node;
						read_node_from_file(curr_node.pointer[i - 1], lbchild_node);

						if (lbchild_node.key_total > MAX_KEY / 2)       //���N  ���ֵܹ������ˣ���һ��
						{
							//child_node starts �ӽڵ�����һ������ֵܵ����һ���ù���
							for (j = child_node.key_total; j > 0; j--)
							{
								child_node.key[j] = child_node.key[j - 1];
								child_node.pointer[j] = child_node.pointer[j - 1];
							}
							child_node.key[0] = curr_node.key[i - 1];
							child_node.pointer[0] = lbchild_node.pointer[lbchild_node.key_total - 1];
							child_node.key_total++;
							//child_node ends

							//lbchild_node starts ���ֵܶ������һ��
							lbchild_node.key_total--;
							//lbchild_node ends

							//curr_node starts ���ڵ�ֻҪ����һ���Լ���һ��key��ok
							curr_node.key[i - 1] = lbchild_node.key[lbchild_node.key_total - 1];
							//curr_node ends

							write_node_to_file(curr_node.pointer[i - 1], lbchild_node);
							write_node_to_file(curr_node.pointer[i], child_node);
							write_node_to_file(curr, curr_node);

						}
						else        //���O	���ֵܲ����ˣ��ϲ�,
						{
							//lbchild_node starts
							//??�ǲ���Ҫ�Ѹ��ڵ���ù���
							//!!���ã���Ϊ��������Ҷ�ڵ�
							//���ֵܰ��ҽڵ�����ж���(����ָ����һ���ָ��)���ù�����<u>��β��Ҫ���ڵ��key����</u>
							for (j = 0; j < child_node.key_total; j++)        //�����ֵܺ��ӽڵ�ϲ�  
							{
								lbchild_node.key[lbchild_node.key_total + j] = child_node.key[j];
								lbchild_node.pointer[lbchild_node.key_total + j] = child_node.pointer[j];
							}
							lbchild_node.key_total += child_node.key_total;

							lbchild_node.pointer[MAX_KEY] = child_node.pointer[MAX_KEY];
							//lbchild_node ends

							//�ͷ�childռ�õĿռ�x.Pointer[i]  

							//curr_node starts
							//���ڵ�������һ��
							for (j = i - 1; j < curr_node.key_total - 1; j++)
							{
								curr_node.key[j] = curr_node.key[j + 1];
								curr_node.pointer[j + 1] = curr_node.pointer[j + 2];
							}

							curr_node.key_total--;
							//curr_node ends

							write_node_to_file(curr_node.pointer[i - 1], lbchild_node);
							write_node_to_file(curr, curr_node);

							i--;

						}

					}
					else        //����ֻ�����ֵ�  
					{
						Node rbchild_node;
						read_node_from_file(curr_node.pointer[i + 1], rbchild_node);

						if (rbchild_node.key_total > MAX_KEY / 2)       //���P  ���ֵܹ������ˣ���һ��
						{
							//curr_node starts
							// ���ڵ�ֻҪ����key�ͺ���
							curr_node.key[i] = rbchild_node.key[0];
							//curr_node ends

							//child_node starts
							// �ӽڵ�ֻҪ�����ұ߽ڵ��һ����ͺ���
							child_node.key[child_node.key_total] = rbchild_node.key[0];
							child_node.pointer[child_node.key_total] = rbchild_node.pointer[0];
							child_node.key_total++;
							//child_node ends

							//rbchild_node starts
							// �ҽڵ�����һ��
							for (j = 0; j < rbchild_node.key_total - 1; j++)
							{
								rbchild_node.key[j] = rbchild_node.key[j + 1];
								rbchild_node.pointer[j] = rbchild_node.pointer[j + 1];
							}
							rbchild_node.key_total--;
							//rbchild_node ends

							write_node_to_file(curr, curr_node);
							write_node_to_file(curr_node.pointer[i + 1], rbchild_node);
							write_node_to_file(curr_node.pointer[i], child_node);

						}
						else        //���Q  �ҽڵ㲻���ˣ���һ���
						{
							//child_node starts
							//�ӽڵ���ҽڵ�ȫ�����ܣ������MAX_KEY��ָ��ҲҪ
							for (j = 0; j < rbchild_node.key_total; j++)
							{
								child_node.key[child_node.key_total + j] = rbchild_node.key[j];
								child_node.pointer[child_node.key_total + j] = rbchild_node.pointer[j];
							}
							child_node.key_total += rbchild_node.key_total;
							child_node.pointer[MAX_KEY] = rbchild_node.pointer[MAX_KEY];
							//child_node ends

							//�ͷ�rbchildռ�õĿռ�x.Pointer[i+1]  

							//curr_node starts
							//���ڵ�������һ�������key[i](Ҳ����ֱ����key[i+1]��ֵ)
							for (j = i; j < curr_node.key_total - 1; j++)
							{
								curr_node.key[j] = curr_node.key[j + 1];
								curr_node.pointer[j + 1] = curr_node.pointer[j + 2];
							}
							curr_node.key_total--;
							//curr_node ends

							write_node_to_file(curr, curr_node);
							write_node_to_file(curr_node.pointer[i], child_node);


						}

					}

				}

			}

			remove_pos(key, curr_node.pointer[i]);
		}
		//����ýڵ�ΪҶ�ڵ㣬��ô�ڸýڵ���û�ҵ��ؼ��֣���˵��b+����û�иùؼ���


	}


}


int main()
{
	//create a new file
	remove("index");
	FILE* tmp = fopen("index", "w");
	fclose(tmp);

	const bool debug = false;
	{
		Bp_tree bt("index");
		bt.build();
		Key_Pointer kp;
		const int total = 500;
		//for (int i = total; i >= 0; --i)
		for (int i = total - 1; i >= 0; --i)
		{
			kp.key = i;
			kp.pointer = i * 100;
			bt.insert(kp);
		}

		if (debug)
		{
			bt.toggle();
			print_index("index", sizeof(Node));

		}

		for (int i = 0; i<total; ++i)
			printf("%d:data:%d\tb_pos:%d\n", i, bt.query(i), bt.query(i, true));
		printf("***************************\n");

		//ɾ��
		for (int i = 0; i < total; ++i)
		{
			//int d = rand() % total;;
			int d = i;

			//??���������Ļ���ȫ������ɾ������
			if (d % 3 == 0)
				continue;

			if (debug)
				printf("deleting %d...\n-----------\n", d);
			bt.remove(d);


			if (debug)
			{
				bt.toggle();
				printf("*****************\n");
				print_index("index", sizeof(Node));
				printf("*****************\n");
			}

		}

		bt.toggle();
		for (int i = 0; i<total; ++i)
			printf("%d:data:%d\tb_pos:%d\n", i, bt.query(i), bt.query(i, true));

		//�ٴβ���
		for (int i = total - 1; i >= 0; --i)
		{
			printf("\tInserting %d...\n", i);
			kp.key = i;
			kp.pointer = i * 100;
			bt.insert(kp);

			if (debug)
			{
				bt.toggle();
				print_index("index", sizeof(Node));

			}
		}


		bt.toggle();
		for (int i = 0; i<total; ++i)
			printf("%d:data:%d\tb_pos:%d\n", i, bt.query(i), bt.query(i, true));
	}


	if (debug)
	{
		print_index("index", sizeof(Node));
	}

}
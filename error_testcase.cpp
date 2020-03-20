void polymorphic(){
	;
}
int polymorphic(float p, float z){
	polymorphic();
	polymorphic(1.0,p);
}
int polymorphic(float p, int z){
	z = 1.0;
	polymorphic();
	polymorphic(1.0,p);
}
int polymorphic(float p, int z, float j){
	p = 1.0;
	polymorphic(1.0,p);
	polymorphic(1.0,p,p);
}
void polymorphic(float p, int z, void j){
	p = 1.0;
	polymorphic(1.0,p);
	polymorphic(1.0,p,p);
}

void array_function(float a[10]){
	;
}
void array_function(float a[10][10]){
	;
}
void array_function(int a[10]){
	;
}
void array_function(int a[10][20]){
	;
}
void fail_cases(){
	;
}
void fail_cases(int p1,int p2,int p3){
	;
}
void fail_cases(int p1,float p2,float p3){
	;
}
void fail_cases(int p1){
	int arr[10.0];
	int a,b,a;
	float b;
	int p1;

	arr[10.0] = 1;
	arr[10] = 1;
	arr[1][10] = 1;
	arr = 1;

	undeclared_function();
	undeclared_var = 10;

	a();
	undeclared_var();

	b = "Hello";
	fail_cases(p1,p1,p1);
	fail_cases(p1,p1,1.0);
	fail_cases(p1,2.0,2.0);

	return 10;
}
void fail_cases(int param){
	;
}
int fail_cases(int param){
	;
}

void printf(int a){
	;
}

void print_functions(){
	int a;
	a = printf("Hello");
	printf();
}

void fail_cases_test(){
	int z1[10];
	int z2[100];
	float z3[10];
	float z4[10][10];

	int a;
	void b;
	a = fail_cases("Hello World!");
	fail_cases(10,10);

	array_function(z1);
	array_function(z2);
	array_function(z3);
	array_function(z4);

	print_functions();
}

int not_main(){
	int a,b;
	int x[1][1];
	float c;
	a = 1;

	a = (a < b);
	b = (a < c);
	b = (a && c);
	a = (c != b);
	
	for(a = 0; a < 10; a++){
		;
	}
}
void main(){
	;
}
int main(){
	int a,b;
	int z[10][20];
	a = 10;	
	array_function(z);
}
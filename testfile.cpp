
void void_function(){
	;
}

int int_function(int a){
	printf("%d\n",a);	
}

float float_function(float a){
	printf("%f\n",a);
}

int array_pass(int a[10][20]){
	printf("%d\n",a[3][4]);
}

void print_array(int a[10]){
	int i;
	printf("[");
	for(i = 0 ; i < 10; i++){
		if(i != 9){
			printf("%d , ",a[i]);
		}else{
			printf("%d",a[i]);
		}
	}
	printf("]\n");
}

void create_fibonacci(int a[10]){
	int i;
	a[0] = (a[1] = 1);
	for(i = 2 ; i < 10; i++){
		a[i] = a[i-1] + a[i-2];
	}
}
int print_fibonacci(int n){
	int a[10],i;
	create_fibonacci(a);
	printf("\n%s\n","Fibonacci Series");
	print_array(a);
	return a[n];
}

int test_while(int a, int b){
	int x;
	while((a < 10) || (b < 5)){
		printf("%s%d,","a: ",a);
		printf("b: %d;\n",b);
		x = a++;
		x = b++;
	}
}

int test_for(int a){
	int x;
	for( x = 0; (a - 10) && 10; a++ ){
		printf("%d\n",a);
	}
}

int factorial(int a){
	int x;
	int result;
	result = 1;
	for( x = 1; x <= a; x++ ){
		result = result*x;
	}
	return result;
}

int factorial_recursive(int a){
	if(a == 0){
		return 1;
	}else{
		;
	}
	return a*factorial_recursive(a-1);
}

int test_if(int a, float p){
	int b;
	b = p;
	printf("%d\n", b);
	if( (a && 10 && (a < 5)) || (b > 2)){
		printf("%s\n","Oops");
	}else{
		;
	}
}

int test_assignment(){
	int a,b,array[12];
	float c;
	c = 1.0;
	b = 2;
	a = (b = c) + 1;
	printf("%d %d %d\n",a,b,c);
	a = (c = 2.5) + 1;
	printf("%d %d %f\n",a,b,c);

	a = 1;
	b = a + (a = 5);
	printf("%d %d \n",a,b);

	array[10] = 0;
	array[1] = 0;

	a = 1;
	array[a] = a = 10;
	printf("%d %d %d\n",a,array[1],array[10]);
}

int test_unary_operations(){
	int a,b;
	;
}

int test_save_registers(){
	int a;
	float b;
	a = 10;
	b = 2.5;
	a = ((a*b)+(a++))*(a+int_function(a));
	printf("%d\n",a);
	return a;
}

int intense_function_helper(int mode){
	int a;
	if(mode == 1){
		a = 10;
		return ((a*a)+(a*a))*(a*a);
	}else{
		a = 10;
		return ((a*a)+(1.0*a*a))/(a*a);
	}
}
int testing_intense_functions(){
	int a;
	a = 1;
	a = intense_function_helper(intense_function_helper(0)) + intense_function_helper(a && (a-1));
	a = a*intense_function_helper(a || (a-1));
	printf("%d\n",a);
}

int search_in_array(int a[10],int val){
	int i;
	for(i = 0; i < 10; i++){
		if(a[i] == val){
			return i;
		}else{
			;
		}
	}
	return -1;
}

int binary_search_in_array(int a[10],int val, int start,int final){
	int middle;
	if(final < start){
		;
	}else{
		if(final == start){
			if(a[start] == val){
				return start;
			}else{
				return -1;
			}
		}else{
			;
		}	
	}
	
	middle = (start + final)/2;
	if(a[middle] == val){
		return middle;
	}else{
		if(a[middle] < val){
			return binary_search_in_array(a,val,middle+1,final);
		}else{
			return binary_search_in_array(a,val,start,middle);
		}
	}
}

int test_array(int a[10]){
	printf("%d\n",a[a[factorial(4)-a[binary_search_in_array(a,21,0,9)]] + 5]);
}

int test_nested_loop(){
	float a[2][3][4];
	int i,j,k;


	for(i = 0; i < 2; i++){
		for(j = 0; j < 3; j++){
			for(k = 0; k < 4; k++){
				a[i][j][k] = 1.0*(i+j+k);
			}
		}
	}

	for(i = 0; i < 2; i++){
		for(j = 0; j < 3; j++){
			for(k = 0; k < 4; k++){
				printf("%d ",a[i][j][k]);
			}
			printf("||");
		}
		printf("\n");
	}
}

int test_infi_nesting(){
	int a,b,c;
	b = 5;
	
	for(a = 0; a < 4; a ++){
		c = 0;
		while(c < 3){
			if(b > 5){
				printf("%d %d %d\n",a,b,c++);
				b = b-1;
			}else{
				printf("%d %d %d\n",a,b++,c++);
			}
		}
	}

}

int test_local_scope(){
	int test_infi_nesting;
	test_infi_nesting = 10;
}

int main(){
	int b,a[10][20],c;
	int fibonacci,fib_array[10];

	void_function();
	int_function(1);
	float_function(10.0);

	c = 10;
	a[3][4] = 100;
	array_pass(a);
	b = factorial(5);
	printf("%d\n",b);
	printf("%d\n",factorial_recursive(5));
	test_if(0,1);
	test_if(4,1);
	test_if(0,3);
	test_assignment();
	test_save_registers();

	fibonacci = 6;
	printf("%d\n",print_fibonacci(6));
	testing_intense_functions();

	create_fibonacci(fib_array);
	print_array(fib_array);
	
	printf("%d\n",search_in_array(fib_array,13));
	printf("%d\n",search_in_array(fib_array,190));

	printf("%d\n",binary_search_in_array(fib_array,13,0,9));
	printf("%d\n",binary_search_in_array(fib_array,190,0,9));

	b = fib_array[binary_search_in_array(fib_array,21,0,9)];
	test_array(fib_array);

	test_nested_loop();
	test_infi_nesting();
}

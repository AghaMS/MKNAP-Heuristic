// Original Code to solve the MKNAP instances fouind at (http://people.brunel.ac.uk/~mastjjb/jeb/orlib/mknapinfo.html)
// Developed by Mohammed Alagha
// June 2014


#include <ilcplex/ilocplex.h>
ILOSTLBEGIN

#include<stdio.h>
#include<vector>
#include<math.h>
#include<time.h>
#include<stdlib.h>
#include<fstream>
using namespace std;
FILE *input;
ofstream MyExcelFile;

int numParticles = 30;
int numColumns;
int numRows;
int cCoef[1100];
int aCoef[50][500];
int b[50];
int rankColumns[500];
int columnsStatus[500];
int globalBest = 0;
int globalParticle;
int globalColumnsStatus[500];
double OptimalValue;
double RCs[500];
double Xs[500];
double Pis[50];
double deltaC[500];
double RCmax;
double RCmin;
double GenericUtilities[500];
double cs[500];
double imputed_cost[500];
double position_X[30][500];
float velocity_V[30][500];
double actual_utility[500];
double generic_utility[30][500];
double Max_Position[30][500];
double Min_Position[30][500];
int particle_best[30][500];
double DeltaC_up[30][500];
double DeltaC_down[30][500];

int list1[500];
int reducedSize;

void readKnapsack(void);
void populate(IloModel model,IloNumVarArray x,IloNumArray colCoef,IloRangeArray allConst);
void initializeParameters();
void initializeParticles();
void changeSolution(double *val,int tempColumnsStatus[],int LHS[]);
int selectColumn(double heuristics[],int tempColumnsStatus[]);
int check_feasible(int f,int LHS[]);
int selectColumnLS(int possibleSelectionsCounter,int possibleSelections[]);
double generateSolution(int particle, int columnsStatus[]);
double localSearch(double val,int tempColumnsStatus[],int LHS[]);
double dropColumns(int tempColumnsStatus[],int LHS[], double value);
double addColumns(int tempColumnsStatus[],int LHS[], double value);
int updateIterationBest( double particleObjective, int particle, double iterationBest, int itBestParticle);
void updatePositions(int itBestParticle, int globalParticle);

void main()
{
	
	time_t startTime = time(NULL);
	clock_t t;
	readKnapsack();
	IloEnv env; // construct the environment
	IloModel model (env); // construct the model
	IloCplex cplex (model); 
	IloNumVarArray x(env);
	IloNumArray colCoef (env);
	IloRangeArray allConst(env);
	IloNumArray dualValues(env), vals(env), vals1(env);
	populate(model,x,colCoef,allConst);
	cplex.exportModel("trial.lp");
	cplex.solve();
	
	double objValue;
	objValue = cplex.getObjValue();
	cout<<"The OF value is "<< objValue <<endl;
	cplex.getDuals(dualValues, allConst);
	cplex.getReducedCosts(vals1, x); 
	cplex.getValues (vals, x);
	for(int j=0;j<numColumns;j++)
		{
		RCs[j]=vals1[j];
		Xs[j]=vals[j];
		//cout<<"RC["<<j<<"] = "<<RCs[j]<<", while X["<<j<<"] = "<<Xs[j]<<endl;
		}
	for(int i=0;i<numRows;i++)
	{
		Pis[i]=dualValues[i];
		//cout<<"Y["<<i<<"] = "<<Pis[i]<<endl;
	}
	for (int seed=4;seed<5;seed++)
		{
			srand(seed);
	initializeParameters();
	initializeParticles();
	int iteration=0;
	float bestValue=0;
	int itBestParticle;
	double globalBestOF = 0;
	
	t = clock();
	cout<<" \n trial number "<<seed<<endl;
	do{	
		double iterationBest= -10000;
		itBestParticle = -1000;
		for(int particle=0;particle<numParticles;particle++)
			{ 
				double particleObjective=0;
				for(int trial=0;trial<1;trial++)
					{				
						double solution=generateSolution(particle,columnsStatus);
						if (solution>particleObjective)
								particleObjective=solution;
						if(solution>bestValue)
							{
								bestValue=solution;
								globalParticle = particle;
								for ( int i = 0; i < numColumns; i++)
									globalColumnsStatus[i] = columnsStatus[i];
							}
						//printf("\n particle=%d trial=%d sol=%f best=%f GlobP=%d iteration=%d",particle,trial, solution,bestValue,globalParticle, iteration);
						//getchar();
					}
				itBestParticle = updateIterationBest( particleObjective,  particle, iterationBest, itBestParticle); 
							if (iteration > 0 && iteration%150==0)
			{
				if ( particle>0 && particle != globalParticle)
					{
						double rand3;
						double rand4;
						rand3 = ((double) rand() / (RAND_MAX+1));
						rand4 = 0.5 + (0.75*rand3);
						for ( int w = 0; w < numColumns; w++)
						Min_Position [particle][w] = (rand4 * imputed_cost[w]);
					}
			}
			}
		updatePositions( itBestParticle,  globalParticle);
		//printf("\n   best=%f  iteration=%d",bestValue, iteration);
		

		iteration++;
	}while(iteration<500);
	t = clock() - t;
	printf(" best=%f  time=%d seconds=%f",bestValue, t, ((float)t)/CLOCKS_PER_SEC);
	}
	getchar();
}
void readKnapsack(void)
{      
		int colcounter = 0;
		input = fopen("mknapcb905.txt","r"); // open file , first is the file name and "r" stands for read
		fscanf(input,"%d", &numColumns); // %d is a format means decimal
		
		fscanf(input,"%d", &numRows); // reads data from the stream "input" and stores them according to the format
		fscanf(input,"%d", &OptimalValue); // scans the best known answer
		
		

		for (int j = 0; j < numColumns; j++) //obtaining column coeff. from the stream
			fscanf(input, "%d", &cCoef[j]);
			
		for (int i = 0; i < numRows; i++)
			for (int j = 0; j < numColumns; j++)
				fscanf(input,"%d",&aCoef[i][j]);

		for (int i = 0; i < numRows; i++)
			fscanf(input,"%d",&b[i]);
			
} // end or reading the knapsack problem
void populate(IloModel model,IloNumVarArray x,IloNumArray colCoef,IloRangeArray allConst)
{
    IloEnv env = model.getEnv();
	for (int j=0; j<numColumns; j++)
	{

		x.add(IloNumVar (env, 0.0, 1.0));
		
		colCoef.add(cCoef[j]);
	}

	model.add(x); // adding variables to the model

	IloObjective objective=IloMaximize(env, IloScalProd(colCoef,x)); // if knapcack
	
	model.add(objective); //adding the OF to the model
	
	for ( int i = 0; i < numRows; i++ )
	{
		IloExpr constraint(env); // defining constraints "building expressions"
		
		for (int j=0; j<numColumns; j++)
			constraint += x[j]*aCoef[i][j];		
		allConst.add( 0<=constraint <= b[i]);
	}

	model.add(allConst);
}
void initializeParticles()
{
	double uniform;
	int fraction = 0.5 * numParticles;
	
	for ( int i = 0; i < numColumns; i++)
		{
			
			position_X[0][i] = cs[i]; 
			velocity_V[0][i] = 0.0;
			generic_utility[0][i] = position_X[0][i] / imputed_cost[i];
			particle_best[0][i] = 0;
			Max_Position [0][i] = (2 * imputed_cost[i]); 
			Min_Position [0][i] = (0.5 * imputed_cost[i]);
			DeltaC_up[0][i] = (Max_Position [0][i]-position_X[0][i]) / 100.0;
			DeltaC_down[0][i] = (position_X[0][i]-Min_Position [0][i]) / 200.0;
//			printf("\n i=%d u=%f imputed=%f",i,generic_utility[0][i],imputed_cost[i]);
//			getchar();
		}
	for(int p=1;p<=fraction;p++)
		{
		for ( int i = 0; i < numColumns; i ++)
			{
			velocity_V[p][i] = 0.0;

			if ( RCs[i] == 0.0 )
				position_X[p][i] = cs[i]; 
			else
				{
				if ( RCs[i] > 0.0)
					position_X[p][i] = ( 1.0 + ((RCs[i]/RCmax)*0.5)) * imputed_cost[i];
				else
					if ( RCs[i] < 0.0)
						position_X[p][i] = ( 0.5 + ((1.0 - (RCs[i]/RCmin))*0.5)) * imputed_cost[i];				
				}
			generic_utility[p][i] = position_X[p][i] / imputed_cost[i];
			particle_best[p][i] = 0;
			Max_Position [p][i] = (2 * imputed_cost[i]); 
			Min_Position [p][i] = (0.5 * imputed_cost[i]);
			DeltaC_up[p][i] = (Max_Position [p][i]-position_X[p][i]) / 100.0;
			DeltaC_down[p][i] = (position_X[p][i]-Min_Position [p][i]) / 200.0;
			
			}
		}


	for(int p=fraction+1;p<numParticles;p++)
		for ( int i = 0; i < numColumns; i++)
			{
			uniform = ((double) rand() / (RAND_MAX+1));		
			if ( RCs[i] >= 0.0)
				position_X[p][i] = cs[i] + deltaC[i]*uniform;
			else
				position_X[p][i] = cs[i] - deltaC[i]*uniform;
			velocity_V[p][i] = 0.0;

			generic_utility[p][i] = position_X[p][i] / imputed_cost[i];
			particle_best[p][i] = 0;
			Max_Position [p][i] = (2.0 * imputed_cost[i]); 
					//Min_Position [a][i] = (rand2 * imputed_cost[i]);
			Min_Position [p][i] = (0.5 * imputed_cost[i]);
			DeltaC_up[p][i] = (Max_Position [p][i]-position_X[p][i]) / 100.0;
			DeltaC_down[p][i] = (position_X[p][i]-Min_Position [p][i]) / 200.0;
			}
}
void initializeParameters()
{
	RCmax=-100000;
	RCmin=10000;
	for ( int i = 0; i < numColumns; i++)
		{
			cs[i] = cCoef[i] - RCs[i];
			if ( RCs[i] >= 0)
			{
				deltaC[i] = cCoef[i] - cs[i];
			}
			else
			{
				deltaC[i] = cs[i] - cCoef[i];
			}
		if(RCs[i]<RCmin)
			RCmin=RCs[i];
		if(RCs[i]>RCmax)
			RCmax=RCs[i];
		}
	for (int i = 0; i < numColumns; i++)
		imputed_cost[i] = 0.0;
	
	for (int i=0;i<numColumns;i++)
		list1[i] = -1;

	for ( int i = 0; i < numColumns; i++)
		{
			for (int j = 0; j < numRows; j++)
			{
				double w = (double)aCoef[j][i];
				imputed_cost[i] += w*Pis[j];
			}
		}
	double utilities[500];
	for ( int i = 0; i < numColumns; i++)
		{
			utilities[i]=cCoef[i] / imputed_cost[i] ;
			GenericUtilities[i]=utilities[i];
			rankColumns[i]=i;
		}
	int dumI;
	double dumD;
	for(int i=0;i<numColumns-1;i++)
		for(int ii=i+1;ii<numColumns;ii++)
			if(utilities[ii]>utilities[i])
				{
					dumD=utilities[i];
					utilities[i]=utilities[ii];
					utilities[ii]=dumD;
					dumI=rankColumns[i];
					rankColumns[i]=rankColumns[ii];
					rankColumns[ii]=dumI;
				}

			int listSize=0;
			int cols[500];
			for ( int i=0;i<numRows;i++)
			{
				double ca_ratio[500];
				for(int j=0;j<numColumns;j++)
				{	ca_ratio[j]= (double) cCoef[j]/(double)aCoef[i][j]; 
					cols[j]=j;
				}
				for (int k=0;k<numColumns;k++)
				{
					int largest = k;
					for (int kk=k+1;kk<numColumns;kk++)
					{
						if (ca_ratio[kk] > ca_ratio[k])
							largest = kk;
					}
					swap(ca_ratio[largest], ca_ratio[k]);
					swap(cols[largest], cols[k]);
				}
				if ( i==0)
					{
						for(int y=0;y<10;y++)
							list1[y]=cols[y];
						listSize=10;
					}
				int u =0;
				for (int a=0;a<10;a++)
					{
						for (int aa=0;aa<listSize;aa++)
							if(cols[a] != list1[aa])
								u=0;
							else
								{
									u=1;
									break;
								}
							if (u==0)
							{
								list1[listSize]=cols[a];
									listSize++;
							}
				}
			}
						
			for (int i=0;i<numColumns;i++)
				{
					int z=0;
					if (RCs[i] >= -400.0)
					{
						for(int j=0;j<listSize;j++)
							if(i != list1[j])
								z=0;
							else
							{
								z=1;
								break;
							}
							if (z==0)
							{
								list1[listSize]=i;
								listSize++;
							}
					}
				}
	reducedSize = listSize;
}
double generateSolution(int particle,int columnsStatus[])
{
	double val=0;
	int LHS[50];
	int tempColumnsStatus[500];
	
	for(int i=0;i<numRows;i++)
		LHS[i]=0;
	double heuristics[500];
	for ( int i = 0; i < numColumns; i++)
		{
			heuristics[i] = pow (generic_utility[particle][i], 4.0);

		}
	for(int i=0;i<numColumns;i++)
		{
			tempColumnsStatus[i]=0;
		}
	int addCheck=0;
	while(addCheck==0)
		{
			int column=selectColumn(heuristics,tempColumnsStatus);
			if(column==100000)
			addCheck=1;
			if(addCheck==0)
				{
					int status = check_feasible(column, LHS);
					if(status==1)
						{
							tempColumnsStatus[column]=1;
							val+=cCoef[column];
						}
					else
						tempColumnsStatus[column]=2;
				}
		}
		for(int i=0;i<numColumns;i++)
			if(tempColumnsStatus[i]==0 || tempColumnsStatus[i]==2)
				tempColumnsStatus[i]=0;
		double newVal = val;//localSearch(val,tempColumnsStatus,LHS);
	for ( int i=0;i<numColumns;i++)
		columnsStatus[i] = tempColumnsStatus[i];
	return newVal;
}
int selectColumn(double heuristics[],int tempColumnsStatus[])
{
	double sum=0;
	int possibleSelection[500];
	int possibleSelectionCounter=0;
	double tempHeuristics[500];
	for ( int i = 0; i < reducedSize; i++)
	{
		int y = list1[i];
		if (tempColumnsStatus[y] ==0)
		{
			possibleSelection[possibleSelectionCounter]=y;
		tempHeuristics[possibleSelectionCounter]=heuristics[y];
		possibleSelectionCounter++;
		sum+=  heuristics[y];
		}
	}
		
		
	if(sum==0)
		return 100000;
	double P[500];
	for ( int i = 0; i < possibleSelectionCounter; i++)
		{
			P[i] = tempHeuristics[i] / sum;
		}

	double cumProb[600];
	cumProb[0]=0;
	for ( int i = 1; i <= possibleSelectionCounter; i++)
		cumProb[i]=cumProb[i-1]+P[i-1];
	double randomNumber = ((double) rand() / (RAND_MAX+1));
	int counter=0;
	while(randomNumber>cumProb[counter])
		counter++;
	counter--;
	if (counter <= 0)
		counter = 0;
	else
	{
		if (counter >= possibleSelectionCounter)
			counter = possibleSelectionCounter;
	}
	return possibleSelection[counter];
}
int check_feasible(int column,int LHS[])
{
	int Fe;
	int feasibility;
	int tempLHS[50];
	//add the chosen variable's constraint coeff. 
	
	for ( int i = 0; i < numRows; i++)
		tempLHS[i]=LHS[i];

	for ( int i = 0; i < numRows; i++)
	{
		feasibility = b[i] - tempLHS[i] - aCoef[i][column];

		 if ( feasibility >= 0)
			 Fe = 1;
		 else
		 {
			 Fe = 0; 
			 break;
		 }
		 
	}
	if(Fe==1)
		for ( int i = 0; i < numRows; i++)
			LHS[i]+=aCoef[i][column];
	return Fe;
}
double localSearch(double val,int tempColumnsStatus[],int LHS[])
{
	double newVal=val;
	int tempColumnsStatusSent[500];
	int LHSsent[50];
	float valueBefore;
	for(int i=0;i<numRows;i++)
		LHSsent[i]=LHS[i];
	for(int i=0;i<numColumns;i++)
		tempColumnsStatusSent[i]=tempColumnsStatus[i];
	for(int counter=0;counter<1000;counter++)
		{	
			valueBefore=val;
			changeSolution(&newVal,tempColumnsStatusSent,LHSsent);
			if(newVal<valueBefore)
				{
					for(int i=0;i<numRows;i++)
						LHSsent[i]=LHS[i];
					for(int i=0;i<numColumns;i++)
						tempColumnsStatusSent[i]=tempColumnsStatus[i];
					newVal=val;
				}	
			else
				{
					val=newVal;
					for(int i=0;i<numRows;i++)
						LHS[i]=LHSsent[i];
					for(int i=0;i<numColumns;i++)
						tempColumnsStatus[i]=tempColumnsStatusSent[i];
				}
		}

	return val;
}
void changeSolution(double *val,int tempColumnsStatus[],int LHS[])
{
	int possibleSelections[500];
	int possibleSelectionsCounter=0;
	for(int i=0;i<numColumns;i++)
		possibleSelections[i]=i;
	possibleSelectionsCounter=numColumns;


	
	for(int i=0;i<4;i++)
		{
			int dumColumn=selectColumnLS(possibleSelectionsCounter,possibleSelections);

			if(tempColumnsStatus[dumColumn]==0)
				{
					tempColumnsStatus[dumColumn]=1;
					for(int row=0;row<numRows;row++)
						LHS[row]+=aCoef[row][dumColumn];
					*val+=cCoef[dumColumn];
				}
			else
				{	
					tempColumnsStatus[dumColumn]=0;
					for(int row=0;row<numRows;row++)
					LHS[row]-=aCoef[row][dumColumn];
					*val-=cCoef[dumColumn];
				}
	for(int col=dumColumn;col<possibleSelectionsCounter-1;col++)
		possibleSelections[col]=possibleSelections[col+1];
	possibleSelectionsCounter--;

	}
	int checkFeasability=0;
	for(int i=0;i<numRows;i++)
	if(LHS[i]>b[i])
		checkFeasability=1;
	double value=*val;
	if(checkFeasability==1)
		value=dropColumns(tempColumnsStatus,LHS,value);
		value=addColumns(tempColumnsStatus,LHS,value);
	*val=value;
}
int selectColumnLS(int possibleSelectionsCounter,int possibleSelections[])
{
	double prob[500];
	double cumProb[600];
	cumProb[0]=0;
	for(int i=0;i<possibleSelectionsCounter;i++)
		{
			prob[i]=1.0/(double)possibleSelectionsCounter;
			cumProb[i+1]=cumProb[i]+prob[i];



	double randomNumber = ((double) rand() / (RAND_MAX+1));
	if (randomNumber <= 0.002)
		return possibleSelections[0];
	
	int counter=0;
	while(randomNumber>cumProb[counter])
		counter++;
	counter--;
	return possibleSelections[counter];
}
double addColumns(int tempColumnsStatus[],int LHS[], double value)
{
	int feasability;
	int possibleAdds[500];
	int possibility[500];
	int DumRows[30];
	for(int i=0;i<numRows;i++)
		DumRows[i]=i;

	int counter=0;
	for(int i=0;i<numColumns;i++)
	{
		int C=rankColumns[i];
		if(tempColumnsStatus[C]==0)
			{
				possibleAdds[counter]=C;
				possibility[counter]=1;
				counter++;
			}
	}
	for(int row=0;row<numRows;row++)
	{
		int smallest=row;
		for(int rowI=row+1;rowI<numRows;rowI++)
		{
			if((b[rowI]-LHS[rowI])<(b[row]-LHS[row]))
				smallest=rowI;
		}
		swap(DumRows[smallest],DumRows[row]);
	}
	int y;
	
	for(int i=0;i<5 && counter>0;i++)
	{	y=DumRows[i];
		for(int j=0;j<counter;j++)
		{	int d= possibleAdds[j];
			if(aCoef[y][j]+LHS[y]>b[y] && possibility[j]==1)
			{
					possibility[j]=0;
					counter--;
			}
		}
	}
		if (counter==0)
			return value;
	
		int v = DumRows[0];
	for(int i=0;i<counter;i++)
		{
			feasability=1;
			if(possibility[i]==1)
			{
				int col=possibleAdds[i];
				if(LHS[v]+aCoef[v][col]>b[v])
					continue;
				feasability=1;
				for(int row=0;row<numRows;row++)
					if(LHS[row]+aCoef[row][col]>b[row])
						feasability=0;
				if(feasability==1)
					{
						value+=cCoef[col];
						tempColumnsStatus[col]=1;
						int tightness[30];
						for(int rowI=0;rowI<numRows;rowI++)
								LHS[rowI]+=aCoef[rowI][col];
					}			
				}	
		}
	return value;
}
double dropColumns(int tempColumnsStatus[],int LHS[], double value)
{
	int columnsSelected[500];
	int numColumnsSelected=0;
	double utilities[500];

	for(int i=numColumns-1;i>=0;i--)
		{
			int DumCol = rankColumns[i];
			if(tempColumnsStatus[DumCol]==1)
			{
				columnsSelected[numColumnsSelected]=DumCol;
				utilities[numColumnsSelected]=GenericUtilities[DumCol];


				numColumnsSelected++;		
			}
		}
		




	int feasability=1;
	int counter=0;
	do{
		int col=columnsSelected[counter];
		//printf("\n before counter=%d col=%d val=%f fes=%d",counter,col,value,feasability);
		value-=cCoef[col];
		tempColumnsStatus[col]=0;
		feasability=0;
		for(int i=0;i<numRows;i++)
			{
				LHS[i]-=aCoef[i][col];
				if(LHS[i]>b[i])
					feasability=1;
			}

	counter++;
	}while(feasability==1);

	return value;
}
int updateIterationBest( double particleObjective, int particle, double iterationBest, int itBestParticle)
{
	if (particleObjective > iterationBest)
	{
		iterationBest = particleObjective;
		itBestParticle = particle;
	}

	return itBestParticle;
}
void updatePositions(int itBestParticle, int globalParticle)
{
	for ( int particle = 0; particle < numParticles; particle++)
		for ( int i = 0; i < numColumns; i++)
			if (particle == globalParticle)
			{
				if (globalColumnsStatus[i] == 1)
					position_X[globalParticle][i] += DeltaC_up[globalParticle][i];
				else
					position_X[globalParticle][i] -= DeltaC_down[globalParticle][i];
				
				if ( position_X[globalParticle][i] > Max_Position [globalParticle][i])
					position_X[globalParticle][i] = Max_Position [globalParticle][i];
		
				if ( position_X[globalParticle][i] < Min_Position [globalParticle][i])
					position_X[globalParticle][i] = Min_Position [globalParticle][i];
		
				generic_utility[globalParticle][i] = position_X[globalParticle][i] / imputed_cost[i];
			}
			else
			{
				double deltaCIteration = position_X[itBestParticle][i] - position_X[particle][i];
				double deltaCGlobal = position_X[globalParticle][i] - position_X[particle][i];
				double velocity = deltaCGlobal + deltaCIteration;
				position_X[particle][i] += velocity;
				if ( position_X[globalParticle][i] > Max_Position [globalParticle][i])
					position_X[globalParticle][i] = Max_Position [globalParticle][i];
				if ( position_X[globalParticle][i] < Min_Position [globalParticle][i])
					position_X[globalParticle][i] = Min_Position [globalParticle][i];
				generic_utility[particle][i] = position_X[particle][i] / imputed_cost[i];
			}

}


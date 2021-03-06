/*
 * Objects.cpp
 *
 *  Created on: Mar 10, 2017
 *      Author: Kourosh Darvish
 */

#include "Objects.hpp"

void pittObjects::Sphere::BoundingBox(void){
	cout<<"Sphere::BoundingBox"<<endl;
	boundingBox[0]=trackedShape.x_est_centroid; //! center_x
	boundingBox[1]=trackedShape.y_est_centroid;//! center_y
	boundingBox[2]=trackedShape.z_est_centroid;//! center_z

	boundingBox[3]=trackedShape.coefficients[3]*2.0*obstacleSafetyFactorX;//! size_x,
	boundingBox[4]=trackedShape.coefficients[3]*2.0*obstacleSafetyFactorY;//! size_y
	boundingBox[5]=trackedShape.coefficients[3]*2.0*obstacleSafetyFactorZ;//! size_z
}

void pittObjects::Sphere::BoundingBall(void){
	cout<<"Sphere::BoundingBall"<<endl;
	boundingBall[0]=trackedShape.x_est_centroid; //! center_x
	boundingBall[1]=trackedShape.y_est_centroid;//! center_y
	boundingBall[2]=trackedShape.z_est_centroid;//! center_z
	boundingBall[3]=trackedShape.coefficients[3]*2.0*obstacleSafetyFactor;//! size_x,
}

bool pittObjects::Sphere::GraspingPosition(void){
	cout<<"Sphere::GraspingPosition"<<endl;
	/*! Grasping Pose is computed from top of the sphere considering
	 * x axis object = x axis gripper ~ PI (3.14) : Roll
	 * y axis object = y axis gripper~ 0 : Pitch
	 * z axis object = z axis gripper~ 0 : Yaw
	 * x center object grasp= x position gripper
	 * y center object grasp= y position gripper
	 * z center object grasp= z position gripper
	 *  */

	float graspPose[6]; float approachingPose[6];
	//RM_: rotation matrix;
	// object CenterFrame rotation matrix, which is parallel to the world frame for the sphere
	Eigen::Matrix3f RM_World2Object;
	for (int i=1;i<4;i++)
		for (int j=1;j<4;j++)
			if (i==j)
				RM_World2Object(i-1,j-1)=1.0;
			else
				RM_World2Object(i-1,j-1)=0.0;

	float objFrame[]={0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	objFrame[3]=trackedShape.x_est_centroid;
	objFrame[4]=trackedShape.y_est_centroid;
	objFrame[5]=trackedShape.z_est_centroid;

	Eigen::Vector3f Vec_ObjectFr2GraspFr, grasping_EulerAngles; // YPR
	Vec_ObjectFr2GraspFr<< 3.14, 0.0, 3.14;
	Eigen::Matrix3f ROtMat_ObjectFr2GraspFr, RotMat_world2Grasping;
	ROtMat_ObjectFr2GraspFr = Eigen::AngleAxisf(Vec_ObjectFr2GraspFr(0), Eigen::Vector3f::UnitZ())
	* Eigen::AngleAxisf(Vec_ObjectFr2GraspFr(1), Eigen::Vector3f::UnitY())
	* Eigen::AngleAxisf(Vec_ObjectFr2GraspFr(2), Eigen::Vector3f::UnitX());

	RotMat_world2Grasping= RM_World2Object*ROtMat_ObjectFr2GraspFr;
	grasping_EulerAngles=RotMat_world2Grasping.eulerAngles(2,1,0);
	cout<<"Eigen::grasping_EulerAngles:\n"<<grasping_EulerAngles<<endl;

	float graspingPose[6];
	graspingPose[0]=grasping_EulerAngles(0); //Y
	graspingPose[1]=grasping_EulerAngles(1);  //P
	graspingPose[2]=grasping_EulerAngles(2);  //R
	graspingPose[3]=objFrame[3];
	graspingPose[4]=objFrame[4];
	graspingPose[5]=objFrame[5];

	vector<string> name;
	name.push_back("graspingPose1");
	Frame tempGrapsingFrame(name,graspingPose);
	objectFrames.push_back(tempGrapsingFrame);
	name.clear();

	approachingPose[0]=graspingPose[0]; //Y
	approachingPose[1]=graspingPose[1];// P
	approachingPose[2]=graspingPose[2];// R

	approachingPose[3]=graspingPose[3]-RotMat_world2Grasping(0,2)*GraspPoseDistance;
	approachingPose[4]=graspingPose[4]-RotMat_world2Grasping(1,2)*GraspPoseDistance;
	approachingPose[5]=graspingPose[5]-RotMat_world2Grasping(2,2)*GraspPoseDistance;

	name.push_back("approachingPose1");
	Frame tempApproachingFrame(name,approachingPose);
	objectFrames.push_back(tempApproachingFrame);
	name.clear();
	return true;
}

void pittObjects::Sphere::FrameSet(void){
	cout<<"Sphere::FrameSet"<<endl;
	cout<<"Frame=> parallel to World Frame"<<endl;

	vector<string> name;
	float tempObjFrame[]={0.0, 0.0, 0.0,trackedShape.x_est_centroid,trackedShape.y_est_centroid,trackedShape.z_est_centroid };
	name.push_back("centerFrame");
	Frame tempFrame(name,tempObjFrame);
	objectFrames.push_back(tempFrame);
	name.clear();

}

// =======================================

void pittObjects::Cylinder::BoundingBox(void){
	cout<<"Cylinder::BoundingBox"<<endl;
	/*!
	 * rrt* method accept boxes just parallel to x,y,z axis of the world
	 * cylinder can be in any direction, based on its principal axis
	 * it is necessary to find a bounding box that is parallel to world axis and encompass the cylinder in all the direction.
	 * consider that the cylinder is symmetrical
	 * map on axis i:
	 * 				m_i=h/2*n_i+r*r_i=h/2*cos(theta)+h/2*cos(90-theta)=h/2*cos(theta)+h/2*sqrt(1-sin(theta)^2)
	 * */

	float height, radius, normalAxis[3];
	radius= trackedShape.coefficients[6];
	height= trackedShape.coefficients[7];
	normalAxis[0]=trackedShape.coefficients[3];
	normalAxis[1]=trackedShape.coefficients[4];
	normalAxis[2]=trackedShape.coefficients[5];

	boundingBox[0]=trackedShape.x_est_centroid;//! center_x
	boundingBox[1]=trackedShape.y_est_centroid;//! center_y
	boundingBox[2]=trackedShape.z_est_centroid;//! center_z

	boundingBox[3]=	(height/2.0)* normalAxis[0]+radius*sqrt(1-normalAxis[0]*normalAxis[0]);
	boundingBox[4]=	(height/2.0)* normalAxis[1]+radius*sqrt(1-normalAxis[1]*normalAxis[1]);
	boundingBox[5]=	(height/2.0)* normalAxis[2]+radius*sqrt(1-normalAxis[2]*normalAxis[2]);

	boundingBox[3]=boundingBox[3]*2.0*obstacleSafetyFactorX;//! size_x,
	boundingBox[4]=boundingBox[4]*2.0*obstacleSafetyFactorY;//! size_y,
	boundingBox[5]=boundingBox[5]*2.0*obstacleSafetyFactorZ;//! size_z,

}

void pittObjects::Cylinder::BoundingBall(void){
	cout<<"Cylinder::BoundingBall"<<endl;
	float height, radius, normalAxis[3], ballRadius;
	radius= trackedShape.coefficients[6];
	height= trackedShape.coefficients[7];
	normalAxis[0]=trackedShape.coefficients[3];
	normalAxis[1]=trackedShape.coefficients[4];
	normalAxis[2]=trackedShape.coefficients[5];

	ballRadius=sqrt(height*height/4+radius*radius);

	boundingBall[0]=trackedShape.x_est_centroid;//! center_x
	boundingBall[1]=trackedShape.y_est_centroid;//! center_y
	boundingBall[2]=trackedShape.z_est_centroid;//! center_z
	boundingBall[3]=ballRadius*2.0*obstacleSafetyFactor;//! Diameter,

}

bool pittObjects::Cylinder::GraspingPosition(void){
	cout<<"Cylinder::GraspingPosition"<<endl;

	float ErrorCorrection[]={0.005, 0.01,0.0};

	float graspPose[6]; float approachingPose[6] , objFrameScrewingPose[6], objFrameFirstGoal[6];
	float FirstGoalDistance=0.05;// the first goal of the moving cylinder in order to collide with the plate

	Eigen::Vector3f Vec_ObjFr2GraspFr, vec_Grasping; //YPR
	Eigen::Matrix3f Rot_Obj2Grasp, Rot_Grasping,RotMat;
	float height, radius, normalAxis[3];


	/*
	Vec_ObjFr2GraspFr(1)=0.0; Vec_ObjFr2GraspFr(2)=0.0; Vec_ObjFr2GraspFr(3)=3.14;
	Rot_Obj2Grasp=Vec_ObjFr2GraspFr.Vect2RPY();
	Rot_Obj2Grasp.PrintMtx("Rot_Obj2Grasp: ");
	Rot_Grasping= RotMat_W2Obj*Rot_Obj2Grasp;

	Rot_Grasping.PrintMtx("Rotation matrix Grasping: ");
	vec_Grasping= Rot_Grasping.RPY2vect();
	vec_Grasping.Transpose().PrintMtx("Grasping Vector (Y -P R): ");

	graspPose[0]=vec_Grasping(1); //Y
	graspPose[1]=-vec_Grasping(2);  //P
	graspPose[2]=vec_Grasping(3);  //R
	graspPose[3]=objFrame[3];
	graspPose[4]=objFrame[4];
	graspPose[5]=objFrame[5];
	cout<<"Grasping vector(1): "<<endl;
	for (int i=0;i<6;i++)
		cout<<graspPose[i]<<" ";
	cout<<endl;

	cout<<"********"<<endl;
	 */



	radius= trackedShape.coefficients[6];
//	height= trackedShape.coefficients[7];
	height= 0.25;

	normalAxis[0]=trackedShape.coefficients[3];
	normalAxis[1]=trackedShape.coefficients[4];
	normalAxis[2]=trackedShape.coefficients[5];
	cout<<"-------------------"<<endl;
	cout<<"RADIUS:"<<radius<<" HEIGHT:"<<height<<endl;
	cout<<"-------------------"<<endl;
	float VERTICAL_THRESHOLD=0.99;
	float VERTICAL_GRASPING_POINT_THRESHOLD=0.08;// 5 cm from top we grasp the object
	/*!
	 * o if the cylinder is vertical:
	 * 	- vertical cylinder means normalAxis[2]>0.99
	 * 		in this case we grasp the cylinder from top like a sphere from top
	 * 	- we grasp it 5 cm from top
	 *
	 * o if the cylinder is not vertical:
	 *  - non vertical cylinder means normalAxis[2]<0.99
	 * 	- we should do some experiments and check for checking sign of biggest normal value, being positive or negative
	 * 		and its relation to success of grasping.
	 *
	 *
	 * */
	float objFrame[]={0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	objFrame[3]=trackedShape.x_est_centroid +ErrorCorrection[0];
	objFrame[4]=trackedShape.y_est_centroid +ErrorCorrection[1];
	objFrame[5]=trackedShape.z_est_centroid +ErrorCorrection[2];

//	if(objFrame[4]>0)
//		objFrame[3]+=0.02;
//	else
//		objFrame[3]+=0.005;

	if (abs(normalAxis[2])> VERTICAL_THRESHOLD)
	{

		if (normalAxis[2]<0)
			for (int i=0;i<3;i++)
				normalAxis[i]=normalAxis[i]*(-1.0);

		graspPose[0]=3.14; //Y
		graspPose[1]=0.0;  //P
		graspPose[2]=3.14;  //R

		graspPose[3]=objFrame[3]+normalAxis[0]*(height/2.0 - VERTICAL_GRASPING_POINT_THRESHOLD);
		graspPose[4]=objFrame[4]+normalAxis[1]*(height/2.0 - VERTICAL_GRASPING_POINT_THRESHOLD);
		graspPose[5]=objFrame[5]+normalAxis[2]*(height/2.0 - VERTICAL_GRASPING_POINT_THRESHOLD);

		approachingPose[0]=graspPose[0]; //Y
		approachingPose[1]=graspPose[1];// P
		approachingPose[2]=graspPose[2];// R

		approachingPose[3]=graspPose[3]+normalAxis[0]*GraspPoseDistance;
		approachingPose[4]=graspPose[4]+normalAxis[1]*GraspPoseDistance;
		approachingPose[5]=graspPose[5]+normalAxis[2]*GraspPoseDistance;

		objFrameScrewingPose[0]=graspPose[0]; //Y
		objFrameScrewingPose[1]=graspPose[1];// P
		objFrameScrewingPose[2]=graspPose[2];// R
		objFrameScrewingPose[3]=objFrame[3]-normalAxis[0]*height/2.0;
		objFrameScrewingPose[4]=objFrame[4]-normalAxis[1]*height/2.0;
		objFrameScrewingPose[5]=objFrame[5]-normalAxis[2]*height/2.0;

		objFrameFirstGoal[0]=graspPose[0]; //Y
		objFrameFirstGoal[1]=graspPose[1];// P
		objFrameFirstGoal[2]=graspPose[2];// R
		objFrameFirstGoal[3]=objFrame[3]-normalAxis[0]*(height/2.0-FirstGoalDistance);
		objFrameFirstGoal[4]=objFrame[4]-normalAxis[1]*(height/2.0-FirstGoalDistance);
		objFrameFirstGoal[5]=objFrame[5]-normalAxis[2]*(height/2.0-FirstGoalDistance);

	}

	else
	{
		float VERTICAL_GRASPING_POINT_THRESHOLD=0.03;// 5 cm from top we grasp the object
		Eigen::Vector3f RefPoint,ObjPoint;
		Eigen::Vector3f X_grasp, zPrime,Y_grasp,Z_grasp, EulerAngles;
		ObjPoint(0)=objFrame[3];ObjPoint(1)= objFrame[4];ObjPoint(2)= objFrame[5];
		RefPoint(0)=0.0; RefPoint(1)=0.0; RefPoint(2)=5.0;

		if (normalAxis[0]>0)
			for (int i=0;i<3;i++)
				normalAxis[i]=normalAxis[i]*(-1.0);

		X_grasp(0)=normalAxis[0];
		X_grasp(1)=normalAxis[1];
		X_grasp(2)=normalAxis[2];

		X_grasp=X_grasp/X_grasp.norm();

		zPrime=(ObjPoint- RefPoint);
		cout<<"zPrime: \n"<<zPrime<<endl;

		zPrime=zPrime/(zPrime.norm());
		cout<<"unit zPrime: \n"<<zPrime<<endl;

		Z_grasp=zPrime-X_grasp*(zPrime.dot(X_grasp));

		Z_grasp=Z_grasp/(Z_grasp.norm());
		cout<<"Z_grasp: \n"<<Z_grasp<<endl;


		Y_grasp=Z_grasp.cross(X_grasp);
		cout<<"Y_grasp: \n"<<Y_grasp<<endl;


		RotMat(0,0)=X_grasp(0); RotMat(0,1)=Y_grasp(0); RotMat(0,2)=Z_grasp(0);
		RotMat(1,0)=X_grasp(1); RotMat(1,1)=Y_grasp(1); RotMat(1,2)=Z_grasp(1);
		RotMat(2,0)=X_grasp(2); RotMat(2,1)=Y_grasp(2); RotMat(2,2)=Z_grasp(2);

		cout<<"RotMat: \n"<<RotMat<<endl;

		EulerAngles=RotMat.eulerAngles(2,1,0);

		cout<<"Euler Angles: \n"<<EulerAngles<<endl;

		graspPose[0]=EulerAngles(0); //Y
		graspPose[1]=EulerAngles(1);  //P
		graspPose[2]=EulerAngles(2);  //R
		graspPose[3]=objFrame[3]+RotMat(0,2)*VERTICAL_GRASPING_POINT_THRESHOLD;
		graspPose[4]=objFrame[4]+RotMat(1,2)*VERTICAL_GRASPING_POINT_THRESHOLD;
		graspPose[5]=objFrame[5]+RotMat(2,2)*VERTICAL_GRASPING_POINT_THRESHOLD;
		cout<<"Grasping vector: "<<endl;
		for (int i=0;i<6;i++)
			cout<<graspPose[i]<<" ";
		cout<<endl;

		approachingPose[0]=graspPose[0]; //Y
		approachingPose[1]=graspPose[1];// P
		approachingPose[2]=graspPose[2];// R

		approachingPose[3]=objFrame[3]-RotMat(0,2)*GraspPoseDistance; //Z direction 0
		approachingPose[4]=objFrame[4]-RotMat(1,2)*GraspPoseDistance;// Z direction 1
		approachingPose[5]=objFrame[5]-RotMat(2,2)*GraspPoseDistance;// Z direction 2

		objFrameScrewingPose[0]=graspPose[0]; //Y
		objFrameScrewingPose[1]=graspPose[1];// P
		objFrameScrewingPose[2]=graspPose[2];// R
		objFrameScrewingPose[3]=objFrame[3]+RotMat(0,0)*height/2.0;
		objFrameScrewingPose[4]=objFrame[4]+RotMat(1,0)*height/2.0;
		objFrameScrewingPose[5]=objFrame[5]+RotMat(2,0)*height/2.0;

		objFrameFirstGoal[0]=graspPose[0]; //Y
		objFrameFirstGoal[1]=graspPose[1];// P
		objFrameFirstGoal[2]=graspPose[2];// R
		objFrameFirstGoal[3]=objFrame[3]-normalAxis[0]*(height/2.0-FirstGoalDistance);
		objFrameFirstGoal[4]=objFrame[4]-normalAxis[1]*(height/2.0-FirstGoalDistance);
		objFrameFirstGoal[5]=objFrame[5]-normalAxis[2]*(height/2.0-FirstGoalDistance);

	}



	vector<string> name;
	name.push_back("graspingPose1");
	Frame temp_graspingFrame(name,graspPose);
	name.clear();

	name.push_back("approachingPose1");
	Frame temp_approachingFrame(name,approachingPose);
	name.clear();

	name.push_back("screwFramePose");
	Frame temp_screwFrame(name,objFrameScrewingPose);
	name.clear();

	name.push_back("FirstGoalFramePose");
	Frame temp_FirstGoal(name,objFrameFirstGoal);
	name.clear();

	objectFrames.push_back(temp_graspingFrame);
	objectFrames.push_back(temp_approachingFrame);
	objectFrames.push_back(temp_screwFrame);
	objectFrames.push_back(temp_FirstGoal);



	//**********************************************
	//  Center Frame Set of the Cylinder
	//**********************************************
	Eigen::Vector3f X_world, Y_world, Z_world, X_cylinder, Y_cylinder , Z_cylinder, EulerAnglesCenter,X_cross;
	double x_cosine;	//! cosine of angle between x_world, x_cylinder, inner product
	double x_sine;	//! sine of angle between x_world, x_cylinder

	Eigen::Matrix3f R, X_cross_skewSym, I,X_cross_skewSym2;
	X_world(0)=1.0; X_world(1)=0.0; X_world(2)=0.0;
	Y_world(0)=0.0; Y_world(1)=1.0; Y_world(2)=0.0;
	Z_world(0)=0.0; Z_world(1)=0.0; Z_world(2)=1.0;

	X_cylinder(0)=trackedShape.coefficients[3];
	X_cylinder(1)=trackedShape.coefficients[4];
	X_cylinder(2)=trackedShape.coefficients[5];

	X_cross=X_world.cross(X_cylinder); //check
	x_sine=X_cross.norm(); //check
	x_cosine=X_world.dot(X_cylinder);

	X_cross_skewSym(0,0)= 0.0;			X_cross_skewSym(0,1)= -X_cross(2);	X_cross_skewSym(0,2)= X_cross(1);
	X_cross_skewSym(1,0)= X_cross(2);	X_cross_skewSym(1,1)= 0.0;			X_cross_skewSym(1,2)= -X_cross(0);
	X_cross_skewSym(2,0)= -X_cross(1);	X_cross_skewSym(2,1)= X_cross(0);	X_cross_skewSym(2,2)= 0.0;
	//	I(1,1)= 1.0; I(1,2)= 0.0; I(1,3)= 0.0;
	//	I(2,1)= 0.0; I(2,2)= 1.0; I(2,3)= 0.0;
	//	I(3,1)= 0.0; I(3,2)= 0.0; I(3,3)= 1.0;
	I.setIdentity();

	if (x_cosine==-1.0)
	{
		R(0,0)=-1.0; R(0,1)= 0.0; R(0,2)= 0.0;
		R(1,0)= 0.0; R(1,1)=-1.0; R(1,2)= 0.0;
		R(2,0)= 0.0; R(2,1)= 0.0; R(2,2)= 1.0;
	}
	else
	{

		X_cross_skewSym2=X_cross_skewSym*X_cross_skewSym;
		for (int i=0;i<3;i++)
			for (int j=0;j<3;j++)
				X_cross_skewSym2(i,j)=X_cross_skewSym2(i,j)*((1.0-x_cosine)/(x_sine*x_sine));

		R=I+X_cross_skewSym+ X_cross_skewSym2;
	}

	EulerAnglesCenter=R.eulerAngles(2,1,0);; //[yaw, pitch, roll]

	float objFrameCenter[6];
	RotMat_World2Obj=R;
	objFrameCenter[0]=EulerAnglesCenter(0); // Y
	objFrameCenter[1]=EulerAnglesCenter(1);// P
	objFrameCenter[2]=EulerAnglesCenter(2); // R
	objFrameCenter[3]=trackedShape.x_est_centroid;
	objFrameCenter[4]=trackedShape.y_est_centroid;
	objFrameCenter[5]=trackedShape.z_est_centroid;


	name.push_back("centerFramePose");
	Frame temp_centerFrame(name,objFrameCenter);
	objectFrames.push_back(temp_centerFrame);
	name.clear();

	return true;

}

void pittObjects::Cylinder::FrameSet(void){
	cout<<"Cylinder::FrameSet"<<endl;
	/*! we assign cylinder axis as x_cylinder
	 * 	We find the rotation matrix from x_world To x_cylinder= Rx.
	 * 	We rotate y, z axis of the world to y_cylinder,z_cylinder using Rx.
	 *
	 * */

	Eigen::Vector3f X_world, Y_world, Z_world, X_cylinder, Y_cylinder , Z_cylinder, EulerAngles,X_cross;
	double x_cosine;	//! cosine of angle between x_world, x_cylinder, inner product
	double x_sine;	//! sine of angle between x_world, x_cylinder

	Eigen::Matrix3f R, X_cross_skewSym, I,X_cross_skewSym2;
	X_world(0)=1.0; X_world(1)=0.0; X_world(2)=0.0;
	Y_world(0)=0.0; Y_world(1)=1.0; Y_world(2)=0.0;
	Z_world(0)=0.0; Z_world(1)=0.0; Z_world(2)=1.0;

	X_cylinder(0)=trackedShape.coefficients[3];
	X_cylinder(1)=trackedShape.coefficients[4];
	X_cylinder(2)=trackedShape.coefficients[5];

	X_cross=X_world.cross(X_cylinder); //check
	x_sine=X_cross.norm(); //check
	x_cosine=X_world.dot(X_cylinder);

	X_cross_skewSym(0,0)= 0.0;			X_cross_skewSym(0,1)= -X_cross(2);	X_cross_skewSym(0,2)= X_cross(1);
	X_cross_skewSym(1,0)= X_cross(2);	X_cross_skewSym(1,1)= 0.0;			X_cross_skewSym(1,2)= -X_cross(0);
	X_cross_skewSym(2,0)= -X_cross(1);	X_cross_skewSym(2,1)= X_cross(0);	X_cross_skewSym(2,2)= 0.0;
	//	I(1,1)= 1.0; I(1,2)= 0.0; I(1,3)= 0.0;
	//	I(2,1)= 0.0; I(2,2)= 1.0; I(2,3)= 0.0;
	//	I(3,1)= 0.0; I(3,2)= 0.0; I(3,3)= 1.0;
	I.setIdentity();

	if (x_cosine==-1.0)
	{
		R(0,0)=-1.0; R(0,1)= 0.0; R(0,2)= 0.0;
		R(1,0)= 0.0; R(1,1)=-1.0; R(1,2)= 0.0;
		R(2,0)= 0.0; R(2,1)= 0.0; R(2,2)= 1.0;
	}
	else
	{

		X_cross_skewSym2=X_cross_skewSym*X_cross_skewSym;
		for (int i=0;i<3;i++)
			for (int j=0;j<3;j++)
				X_cross_skewSym2(i,j)=X_cross_skewSym2(i,j)*((1.0-x_cosine)/(x_sine*x_sine));

		R=I+X_cross_skewSym+ X_cross_skewSym2;
	}

	EulerAngles=R.eulerAngles(2,1,0);; //[yaw, pitch, roll]

	float objFrame[6];
	RotMat_World2Obj=R;
	objFrame[0]=EulerAngles(0); // Y
	objFrame[1]=EulerAngles(1);// P
	objFrame[2]=EulerAngles(2); // R
	objFrame[3]=trackedShape.x_est_centroid;
	objFrame[4]=trackedShape.y_est_centroid;
	objFrame[5]=trackedShape.z_est_centroid;

//	vector<string> name;
//	name.push_back("centerFramePose");
//	Frame temp_centerFrame(name,objFrame);
//	objectFrames.push_back(temp_centerFrame);
//	name.clear();

}

// =======================================

void pittObjects::Cone::BoundingBox(void){
	cout<<"Cone::BoundingBox"<<endl;

}

void pittObjects::Cone::BoundingBall(void){
	cout<<"Cone::BoundingBall"<<endl;
}

bool pittObjects::Cone::GraspingPosition(void){
	cout<<"Cone::GraspingPosition"<<endl;
	float graspPose[6]; float approachingPose[6];
	return true;
}

void pittObjects::Cone::FrameSet(void){
	cout<<"Cone::FrameSet"<<endl;


}

// =======================================
void pittObjects::Plane::BoundingBox(void){
	cout<<"Plane::BoundingBox"<<endl;

}

void pittObjects::Plane::BoundingBall(void){
	cout<<"Plane::BoundingBall"<<endl;
}

bool pittObjects::Plane::GraspingPosition(void){
	cout<<"Plane::GraspingPosition"<<endl;

	cout<<"Z (Y)    r            r          "<<endl;
	cout<<"^     p4 ^  --------- ^ p2       "<<endl;
	cout<<"|        | |--> b <--||          "<<endl;
	cout<<"|     p3    ---------   p1         "<<endl;
	cout<<" ----> Y (X)                         "<<endl;

	float ErrorCorrection[]={0.0, 0.01,0.0};
	/*!
		1- find the two point on the left for the left arm, and the two point on the right for the right arm
		2- Find the grasping position for the left and right arm based on the the two left/right points, and maybe also center point
		3- find the g and r vectors of the grasping pose base on upper figure
		4- find the b vector using cross product
		5- find the yaw, pitch, roll of the grasping frame using rotation matrix
		6- add the found frames to the frame vector
	 */

	// vertices 1 and 3 are in front of each other. vertices 2 and 4 are in front of each other.

	float GRASPING_DIS=0.05;// 5 cm from the plate border the grasping pose and the approaching pose
	float PLANE_THICKNESS=0.02;
	//rectangle threshold
	double RecThreshold=0.05;



	vector<vector<float>> verticesOld, vertices;
	vector<float> vertex, center, planeCoef;
	// [0 1 2 3]: a, b,c,d
	vertex.push_back(trackedShape.coefficients[4]+ErrorCorrection[0]);
	vertex.push_back(trackedShape.coefficients[5]+ErrorCorrection[1]);
	vertex.push_back(trackedShape.coefficients[6]+ErrorCorrection[2]);
	verticesOld.push_back(vertex);
	vertex.clear();

	vertex.push_back(trackedShape.coefficients[7]+ErrorCorrection[0]);
	vertex.push_back(trackedShape.coefficients[8]+ErrorCorrection[1]);
	vertex.push_back(trackedShape.coefficients[9]+ErrorCorrection[2]);
	verticesOld.push_back(vertex);
	vertex.clear();

	vertex.push_back(trackedShape.coefficients[10]+ErrorCorrection[0]);
	vertex.push_back(trackedShape.coefficients[11]+ErrorCorrection[1]);
	vertex.push_back(trackedShape.coefficients[12]+ErrorCorrection[2]);
	verticesOld.push_back(vertex);
	vertex.clear();

	vertex.push_back(trackedShape.coefficients[13]+ErrorCorrection[0]);
	vertex.push_back(trackedShape.coefficients[14]+ErrorCorrection[1]);
	vertex.push_back(trackedShape.coefficients[15]+ErrorCorrection[2]);
	verticesOld.push_back(vertex);
	vertex.clear();

	///////////////////////////////////////////////
	///////////////////////////////////////////////
	// check if at least three points are theoretically can make a rectangle, if not return false;
/*	vector<float> foundVertex;foundVertex.resize(3,0.0);
	vector<vector<float>> foundVertices;foundVertices.resize(4,foundVertex);


	for(int i=0;i<3;i++)
	{
		foundVertices[1][i]=verticesOld[0][i]+(verticesOld[2][i]-verticesOld[3][i]); // point 2
		foundVertices[2][i]=verticesOld[1][i]+(verticesOld[3][i]-verticesOld[0][i]); // point 3
		foundVertices[3][i]=verticesOld[2][i]+(verticesOld[0][i]-verticesOld[1][i]); // point 4
		foundVertices[0][i]=verticesOld[3][i]+(verticesOld[1][i]-verticesOld[2][i]); // point 1
	}

	vector<int>RectangleIndices;
	for(int i=0;i<4;i++)
	{
		bool closeness=CheckPoints(foundVertices[i],verticesOld[i], RecThreshold);
		if(closeness==false)
		{
			RectangleIndices.push_back(i);
		}
	}

	cout<<"RectangleIndices.size():"<<RectangleIndices.size()<<endl;
	if(RectangleIndices.size()==0)
	{}
	else if(RectangleIndices.size()==1)
	{
		cout<<"vertex replacement"<<endl;
		cout<<"old: "<<RectangleIndices[0]<<": "<<verticesOld[RectangleIndices[0]][0]<<" "<<verticesOld[RectangleIndices[0]][1]<<" "<<verticesOld[RectangleIndices[0]][2]<<endl;
		cout<<"new: "<<RectangleIndices[0]<<": "<<foundVertices[RectangleIndices[0]][0]<<" "<<foundVertices[RectangleIndices[0]][1]<<" "<<foundVertices[RectangleIndices[0]][2]<<endl;
		verticesOld[RectangleIndices[0]]=foundVertices[RectangleIndices[0]];


	}
	else
	{
		return false;
	}*/

	///////////////////////////////////////////////
	///////////////////////////////////////////////


	center.push_back(trackedShape.x_pc_centroid+ErrorCorrection[0]);//x
	center.push_back(trackedShape.y_pc_centroid+ErrorCorrection[1]);//y
	center.push_back(trackedShape.z_pc_centroid+ErrorCorrection[2]);//z
	for(int i=0;i<4;i++)
		planeCoef.push_back(trackedShape.coefficients[i]); //a,b,c,d

	cout<<"plane coef: "<<planeCoef[0]<<" "<<planeCoef[1]<<" "<<planeCoef[2]<<" "<<planeCoef[3]<<" "<<endl;
	// normal of the plate is toward -X:
	if (planeCoef[0]>0)
		for(int i=0;i<4;i++)
			planeCoef[i]=-1.0*planeCoef[i];
	int index1=1, index2=2, index3=0;

	cout<<"plane coef: "<<planeCoef[0]<<" "<<planeCoef[1]<<" "<<planeCoef[2]<<" "<<planeCoef[3]<<" "<<endl;
	// if the plate is horizontal, make the normal toward -Z
	if(planeCoef[2]>0.9)
	{
		for(int i=0;i<4;i++)
			planeCoef[i]=-1.0*planeCoef[i];
		index1=0, index2=1, index3=2;
	}
	else if(planeCoef[2]<-0.9)
	{index1=0, index2=1, index3=2;}

	cout<<"plane coef: "<<planeCoef[0]<<" "<<planeCoef[1]<<" "<<planeCoef[2]<<" "<<planeCoef[3]<<" "<<endl;


	// make vertices ordered based on figure above:
	vertex.resize(3,0.0);
	vertices.resize(4,vertex);
	for(int i=0;i<verticesOld.size();i++)
	{
		if(verticesOld[i][index1]> center[index1] && verticesOld[i][index2]< center[index2])
		{
			cout<<"1"<<endl;
			vertices[0]=verticesOld[i];
		}
		else if(verticesOld[i][index1]> center[index1] && verticesOld[i][index2]> center[index2])
		{
			cout<<"2"<<endl;
			vertices[1]=verticesOld[i];
		}
		else if(verticesOld[i][index1]< center[index1] && verticesOld[i][index2]< center[index2])
		{
			cout<<"3"<<endl;
			vertices[2]=verticesOld[i];
		}
		else if(verticesOld[i][index1]< center[index1] && verticesOld[i][index2]> center[index2])
		{
			cout<<"4"<<endl;
			vertices[3]=verticesOld[i];
		}
		else
			cout<<"Error in vertices and center of plane"<<endl;
	}

	cout<<"center: "<<center[0]<<" "<<center[1]<<" "<<center[2]<<endl;
	cout<<"vertices old:"<<endl;
	for(int i=0;i<4;i++)
	{	for (int j=0;j<3;j++)
		cout<<verticesOld[i][j]<<" ";
	cout<<endl;
	}
	cout<<"vertices:"<<endl;
	for(int i=0;i<4;i++)
	{	for (int j=0;j<3;j++)
			cout<<vertices[i][j]<<" ";
		cout<<endl;
	}

	Eigen::Vector3f gp1,gp2;// grasping Position 1 (p1+p2/2) and 2 (p3+p4/2).
	Eigen::Vector3f graspingPose1, graspingPose2, approachingPose1, approachingPose2;//positions: graspingPose =gp+ GRASPING_DIS* ZDir, approachingPose =gp - GRASPING_DIS* ZDir

	for (int i=0;i<3;i++)
		gp1(i)= (vertices[0][i]+vertices[1][i])/2.0 ;
	for (int i=0;i<3;i++)
		gp2(i)=(vertices[2][i]+vertices[3][i])/2.0 ;


	Eigen::Vector3f X1_grasp, Y1_grasp,Z1_grasp, EulerAngles1, X2_grasp, Y2_grasp,Z2_grasp, EulerAngles2, XCenter_grasp, YCenter_grasp,ZCenter_grasp, EulerAnglesCenter;

	for(int i=0;i<3;i++)
	{
		X1_grasp(i)=((vertices[1][i]-vertices[0][i])+(vertices[3][i]-vertices[2][i]))/2.0;
		X2_grasp(i)=X1_grasp(i);
	}
	X2_grasp=X2_grasp/X2_grasp.norm();
	X1_grasp=X1_grasp/X1_grasp.norm();

	for(int i=0;i<3;i++)
	{
		Y1_grasp(i)=planeCoef[i];
		Y2_grasp(i)=-planeCoef[i];
	}

	Y1_grasp=Y1_grasp/Y1_grasp.norm();
	Y2_grasp=Y2_grasp/Y2_grasp.norm();

	Z1_grasp=X1_grasp.cross(Y1_grasp);
	Z2_grasp=X2_grasp.cross(Y2_grasp);

	XCenter_grasp=X1_grasp; // check later
	YCenter_grasp=Y1_grasp;
	ZCenter_grasp=Z1_grasp;

	graspingPose1 =gp1+ GRASPING_DIS* Z1_grasp;
	approachingPose1 =gp1- GRASPING_DIS* Z1_grasp;

	graspingPose2 =gp2+ GRASPING_DIS* Z2_grasp;
	approachingPose2 =gp2- GRASPING_DIS* Z2_grasp;

	// adjust the points based on the plane thickness
	// Y1_grasp: equal to the normal of the plane in '-X' direction
	graspingPose1 		=graspingPose1 - PLANE_THICKNESS* Y1_grasp;
	approachingPose1	=approachingPose1  - PLANE_THICKNESS* Y1_grasp;

	cout<<"graspingPose2: "<<graspingPose2(0)<<" "<<graspingPose2(1)<<" "<<graspingPose2(2)<<endl;
	cout<<"approachingPose2: "<<approachingPose2(0)<<" "<<approachingPose2(1)<<" "<<approachingPose2(2)<<endl;
	graspingPose2 =graspingPose2 - PLANE_THICKNESS* Y1_grasp;
	approachingPose2 =approachingPose2- PLANE_THICKNESS* Y1_grasp;

	cout<<"graspingPose2: "<<graspingPose2(0)<<" "<<graspingPose2(1)<<" "<<graspingPose2(2)<<endl;
	cout<<"approachingPose2: "<<approachingPose2(0)<<" "<<approachingPose2(1)<<" "<<approachingPose2(2)<<endl;

	Eigen::Matrix3f RotMat1, RotMat2, RotMatCenter;
	float angle=00.0*3.1415/180.0;
	Eigen::Matrix3f rot1(Eigen::AngleAxisf(angle,Eigen::Vector3f::UnitY())), rot2(Eigen::AngleAxisf(angle,Eigen::Vector3f::UnitY()));
//	Eigen::Matrix3f t=tf.rotation();

//	cout<<"Y1_grasp: "<<endl;
//	cout<<Y1_grasp(0)<<" "<<Y1_grasp(1)<<" "<<Y1_grasp(2)<<endl;
//	cout<<endl;
//
//	cout<<"Transformation MAtrix: rot1: "<<endl;
//	cout<<rot1(0,0)<<" "<<rot1(0,1)<<" "<<rot1(0,2)<<endl;
//	cout<<rot1(1,0)<<" "<<rot1(1,1)<<" "<<rot1(1,2)<<endl;
//	cout<<rot1(2,0)<<" "<<rot1(2,1)<<" "<<rot1(2,2)<<endl;
//	cout<<endl;
//	cout<<"Transformation MAtrix: rot2: "<<endl;
//	cout<<rot2(0,0)<<" "<<rot2(0,1)<<" "<<rot2(0,2)<<endl;
//	cout<<rot2(1,0)<<" "<<rot2(1,1)<<" "<<rot2(1,2)<<endl;
//	cout<<rot2(2,0)<<" "<<rot2(2,1)<<" "<<rot2(2,2)<<endl;
//	cout<<endl;

	RotMat1(0,0)=X1_grasp(0); RotMat1(0,1)=Y1_grasp(0); RotMat1(0,2)=Z1_grasp(0);
	RotMat1(1,0)=X1_grasp(1); RotMat1(1,1)=Y1_grasp(1); RotMat1(1,2)=Z1_grasp(1);
	RotMat1(2,0)=X1_grasp(2); RotMat1(2,1)=Y1_grasp(2); RotMat1(2,2)=Z1_grasp(2);

	RotMat2(0,0)=X2_grasp(0); RotMat2(0,1)=Y2_grasp(0); RotMat2(0,2)=Z2_grasp(0);
	RotMat2(1,0)=X2_grasp(1); RotMat2(1,1)=Y2_grasp(1); RotMat2(1,2)=Z2_grasp(1);
	RotMat2(2,0)=X2_grasp(2); RotMat2(2,1)=Y2_grasp(2); RotMat2(2,2)=Z2_grasp(2);

	RotMatCenter=RotMat1;

//	cout<<"Transformation MAtrix: RotMat1: "<<endl;
//	cout<<RotMat1(0,0)<<" "<<RotMat1(0,1)<<" "<<RotMat1(0,2)<<endl;
//	cout<<RotMat1(1,0)<<" "<<RotMat1(1,1)<<" "<<RotMat1(1,2)<<endl;
//	cout<<RotMat1(2,0)<<" "<<RotMat1(2,1)<<" "<<RotMat1(2,2)<<endl;
//	cout<<endl;

	RotMat1=RotMat1*rot1;
	RotMat2=RotMat2*rot2;

//	cout<<"Transformation MAtrix: RotMat1: "<<endl;
//	cout<<RotMat1(0,0)<<" "<<RotMat1(0,1)<<" "<<RotMat1(0,2)<<endl;
//	cout<<RotMat1(1,0)<<" "<<RotMat1(1,1)<<" "<<RotMat1(1,2)<<endl;
//	cout<<RotMat1(2,0)<<" "<<RotMat1(2,1)<<" "<<RotMat1(2,2)<<endl;
//	cout<<endl;

	EulerAngles1		=RotMat1.eulerAngles(2,1,0);
	EulerAngles2		=RotMat2.eulerAngles(2,1,0);
	EulerAnglesCenter	=RotMatCenter.eulerAngles(2,1,0);

	float graspPose1[6], graspPose2[6], graspPoseCenter[6], approachPose1[6], approachPose2[6];

	graspPose1[0]=EulerAngles1(0); //Y
	graspPose1[1]=EulerAngles1(1);  //P
	graspPose1[2]=EulerAngles1(2);  //R
	graspPose1[3]=graspingPose1(0);
	graspPose1[4]=graspingPose1(1);
	graspPose1[5]=graspingPose1(2);

	graspPose2[0]=EulerAngles2(0); //Y
	graspPose2[1]=EulerAngles2(1);  //P
	graspPose2[2]=EulerAngles2(2);  //R
	graspPose2[3]=graspingPose2(0);
	graspPose2[4]=graspingPose2(1);
	graspPose2[5]=graspingPose2(2);

	approachPose1[0]=EulerAngles1(0); //Y
	approachPose1[1]=EulerAngles1(1);  //P
	approachPose1[2]=EulerAngles1(2);  //R
	approachPose1[3]=approachingPose1(0);
	approachPose1[4]=approachingPose1(1);
	approachPose1[5]=approachingPose1(2);

	approachPose2[0]=EulerAngles2(0); //Y
	approachPose2[1]=EulerAngles2(1);  //P
	approachPose2[2]=EulerAngles2(2);  //R
	approachPose2[3]=approachingPose2(0);
	approachPose2[4]=approachingPose2(1);
	approachPose2[5]=approachingPose2(2);

	graspPoseCenter[0]=EulerAnglesCenter(0); //Y
	graspPoseCenter[1]=EulerAnglesCenter(1);  //P
	graspPoseCenter[2]=EulerAnglesCenter(2);  //R
	graspPoseCenter[3]=center[0];
	graspPoseCenter[4]=center[1];
	graspPoseCenter[5]=center[2];


	vector<string> name;
	name.push_back("centerFramePose");
	Frame tempCenter(name,graspPoseCenter);
	objectFrames.push_back(tempCenter);
	name.clear();

	name.push_back("approachingPose1");
	Frame tempap1(name,approachPose1);
	objectFrames.push_back(tempap1);
	name.clear();

	name.push_back("graspingPose1");
	Frame tempgp1(name,graspPose1);
	objectFrames.push_back(tempgp1);
	name.clear();

	name.push_back("approachingPose2");
	Frame tempap2(name,approachPose2);
	objectFrames.push_back(tempap2);
	name.clear();

	name.push_back("graspingPose2");
	Frame tempgp2(name,graspPose2);
	objectFrames.push_back(tempgp2);
	name.clear();




	// screw poses:
	float SCREW_DIS=0.05, SCREW_LENGTH=0.03; // the screw distance from the borders of the plate are 5 cm

	// we use the frames on the center frame (= gp1 frame) in order to find the vectors to move from vertices of the plate to the screw poses
	Eigen::Vector3f screw1, screw2, screw3, screw4, approachScrew1, approachScrew2, approachScrew3, approachScrew4 ;// grasping Position 1 (p1+p2/2) and 2 (p3+p4/2).

	screw1={vertices[0][0], vertices[0][1],vertices[0][2]};
	screw2={vertices[1][0], vertices[1][1],vertices[1][2]};
	screw3={vertices[2][0], vertices[2][1],vertices[2][2]};
	screw4={vertices[3][0], vertices[3][1],vertices[3][2]};

	screw1=screw1+SCREW_DIS* X1_grasp; screw1=screw1+SCREW_DIS* Z1_grasp; screw1=screw1-SCREW_LENGTH* Y1_grasp;
	screw2=screw2-SCREW_DIS* X1_grasp; screw2=screw2+SCREW_DIS* Z1_grasp; screw2=screw2-SCREW_LENGTH* Y1_grasp;
	screw3=screw3+SCREW_DIS* X1_grasp; screw3=screw3-SCREW_DIS* Z1_grasp; screw3=screw3-SCREW_LENGTH* Y1_grasp;
	screw4=screw4-SCREW_DIS* X1_grasp; screw4=screw4-SCREW_DIS* Z1_grasp; screw4=screw4-SCREW_LENGTH* Y1_grasp;

	approachScrew1=screw1; approachScrew2=screw2; approachScrew3=screw3; approachScrew4=screw4;

	approachScrew1=approachScrew1-2.0*SCREW_LENGTH* Y1_grasp;
	approachScrew2=approachScrew2-2.0*SCREW_LENGTH* Y1_grasp;
	approachScrew3=approachScrew3-2.0*SCREW_LENGTH* Y1_grasp;
	approachScrew4=approachScrew4-2.0*SCREW_LENGTH* Y1_grasp;

	Eigen::Vector3f screw1Euler, screw2Euler, screw3Euler, screw4Euler; // Pi,0,PI

	cout<<"X: "<<X1_grasp(0)<<" "<<X1_grasp(1)<<" "<<X1_grasp(2)<<endl;
	cout<<"Y: "<<Y1_grasp(0)<<" "<<Y1_grasp(1)<<" "<<Y1_grasp(2)<<endl;
	cout<<"Z: "<<Z1_grasp(0)<<" "<<Z1_grasp(1)<<" "<<Z1_grasp(2)<<endl;

	cout<<"screw 1: "<<screw1(0)<<" "<<screw1(1)<<" "<<screw1(2)<<endl;
	cout<<"screw 2: "<<screw2(0)<<" "<<screw2(1)<<" "<<screw2(2)<<endl;
	cout<<"screw 3: "<<screw3(0)<<" "<<screw3(1)<<" "<<screw3(2)<<endl;
	cout<<"screw 4: "<<screw4(0)<<" "<<screw4(1)<<" "<<screw4(2)<<endl;

	float screw1Pose[6], screw2Pose[6], screw3Pose[6], screw4Pose[6];
	float approachScrew1Pose[6], approachScrew2Pose[6], approachScrew3Pose[6], approachScrew4Pose[6];
	//3.14 0 3.14
	screw1Pose[0]=3.14; //Y
	screw1Pose[1]=0.0;  //P
	screw1Pose[2]=3.14;  //R
	screw1Pose[3]=screw1(0);
	screw1Pose[4]=screw1(1);
	screw1Pose[5]=screw1(2);

	screw2Pose[0]=3.14; //Y
	screw2Pose[1]=0.0;  //P
	screw2Pose[2]=3.14;  //R
	screw2Pose[3]=screw2(0);
	screw2Pose[4]=screw2(1);
	screw2Pose[5]=screw2(2);

	screw3Pose[0]=3.14; //Y
	screw3Pose[1]=0.0;  //P
	screw3Pose[2]=3.14;  //R
	screw3Pose[3]=screw3(0);
	screw3Pose[4]=screw3(1);
	screw3Pose[5]=screw3(2);

	screw4Pose[0]=3.14; //Y
	screw4Pose[1]=0.0;  //P
	screw4Pose[2]=3.14;  //R
	screw4Pose[3]=screw4(0);
	screw4Pose[4]=screw4(1);
	screw4Pose[5]=screw4(2);

	approachScrew1Pose[0]=3.14; //Y
	approachScrew1Pose[1]=0.0;  //P
	approachScrew1Pose[2]=3.14;  //R
	approachScrew1Pose[3]=approachScrew1(0);
	approachScrew1Pose[4]=approachScrew1(1);
	approachScrew1Pose[5]=approachScrew1(2);

	approachScrew2Pose[0]=3.14; //Y
	approachScrew2Pose[1]=0.0;  //P
	approachScrew2Pose[2]=3.14;  //R
	approachScrew2Pose[3]=approachScrew2(0);
	approachScrew2Pose[4]=approachScrew2(1);
	approachScrew2Pose[5]=approachScrew2(2);

	approachScrew3Pose[0]=3.14; //Y
	approachScrew3Pose[1]=0.0;  //P
	approachScrew3Pose[2]=3.14;  //R
	approachScrew3Pose[3]=approachScrew3(0);
	approachScrew3Pose[4]=approachScrew3(1);
	approachScrew3Pose[5]=approachScrew3(2);

	approachScrew4Pose[0]=3.14; //Y
	approachScrew4Pose[1]=0.0;  //P
	approachScrew4Pose[2]=3.14;  //R
	approachScrew4Pose[3]=approachScrew4(0);
	approachScrew4Pose[4]=approachScrew4(1);
	approachScrew4Pose[5]=approachScrew4(2);


	name.push_back("screwPose");
	name.push_back("screwPose1");
	Frame tempScrew1(name,screw1Pose);
	objectFrames.push_back(tempScrew1);
	name.clear();

	name.push_back("screwPose");
	name.push_back("screwPose2");
	Frame tempScrew2(name,screw2Pose);
	objectFrames.push_back(tempScrew2);
	name.clear();

	name.push_back("screwPose");
	name.push_back("screwPose3");
	Frame tempScrew3(name,screw3Pose);
	objectFrames.push_back(tempScrew3);
	name.clear();

	name.push_back("screwPose");
	name.push_back("screwPose4");
	Frame tempScrew4(name,screw4Pose);
	objectFrames.push_back(tempScrew4);
	name.clear();

	name.push_back("approachScrewPose");
	name.push_back("approachScrewPose1");
	Frame tempApproachScrew1(name,approachScrew1Pose);
	objectFrames.push_back(tempApproachScrew1);
	name.clear();

	name.push_back("approachScrewPose");
	name.push_back("approachScrewPose2");
	Frame tempApproachScrew2(name,approachScrew2Pose);
	objectFrames.push_back(tempApproachScrew2);
	name.clear();

	name.push_back("approachScrewPose");
	name.push_back("approachScrewPose3");
	Frame tempApproachScrew3(name,approachScrew3Pose);
	objectFrames.push_back(tempApproachScrew3);
	name.clear();

	name.push_back("approachScrewPose");
	name.push_back("approachScrewPose4");
	Frame tempApproachScrew4(name,approachScrew4Pose);
	objectFrames.push_back(tempApproachScrew4);
	name.clear();


	float vertexPose[]={0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

	vertexPose[3]=vertices[0][0];vertexPose[4]=vertices[0][1];vertexPose[5]=vertices[0][2];
	name.push_back("VertexPose");
	name.push_back("VertexPose1");
	Frame tempVertex1(name,vertexPose);
	objectFrames.push_back(tempVertex1);
	name.clear();

	vertexPose[3]=vertices[1][0];vertexPose[4]=vertices[1][1];vertexPose[5]=vertices[1][2];
	name.push_back("VertexPose");
	name.push_back("VertexPose2");
	Frame tempVertex2(name,vertexPose);
	objectFrames.push_back(tempVertex2);
	name.clear();

	vertexPose[3]=vertices[2][0];vertexPose[4]=vertices[2][1];vertexPose[5]=vertices[2][2];
	name.push_back("VertexPose");
	name.push_back("VertexPose3");
	Frame tempVertex3(name,vertexPose);
	objectFrames.push_back(tempVertex3);
	name.clear();

	vertexPose[3]=vertices[3][0];vertexPose[4]=vertices[3][1];vertexPose[5]=vertices[3][2];
	name.push_back("VertexPose");
	name.push_back("VertexPose4");
	Frame tempVertex4(name,vertexPose);
	objectFrames.push_back(tempVertex4);
	name.clear();









	return true;
}

void pittObjects::Plane::FrameSet(void){
	cout<<"Plane::FrameSet"<<endl;
	float teta;
	float normalVec[3];
	//	float worldvector[3];
	//	float planeSize[2];
	normalVec[0]=trackedShape.coefficients[0];//a
	normalVec[1]=trackedShape.coefficients[1];//b
	normalVec[2]=trackedShape.coefficients[2];//c
	//	worldvector[0]=0.0;
	//	worldvector[1]=0.0;
	//	worldvector[2]=1.0;

	float cos_teta=normalVec[0]/(sqrt(normalVec[0]*normalVec[0]+normalVec[1]*normalVec[1]+normalVec[2]*normalVec[2]));
	if (normalVec[1]>0.0)
		teta=acos(cos_teta);
	else
		teta=-1.0*acos(cos_teta);

	objFrame[0]=teta;	//yaw
	objFrame[1]=0.0;	//pitch
	objFrame[2]=0.0;	//roll
	objFrame[3]=trackedShape.x_pc_centroid;		//x
	objFrame[4]=trackedShape.y_pc_centroid;	 	//y
	objFrame[5]=trackedShape.z_pc_centroid-0.02;		//z

//	class Frame tempCenter("CenterFrame",objFrame);
//	objectFrames.push_back(tempCenter);

};

// =======================================

void pittObjects::Unknown::BoundingBox(void){
	cout<<"Unknown::BoundingBox"<<endl;


}

void pittObjects::Unknown::BoundingBall(void){
	cout<<"Unknown::BoundingBall"<<endl;

}

bool pittObjects::Unknown::GraspingPosition(void){
	cout<<"Unknown::GraspingPosition"<<endl;
	return true;
}
void pittObjects::Unknown::FrameSet(void){
	cout<<"Unknown::FrameSet"<<endl;

}

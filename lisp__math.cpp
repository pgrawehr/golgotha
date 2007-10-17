/********************************************************************** <BR>
   This file is part of Crack dot Com's free source code release of
   Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
   information about compiling & licensing issues visit this URL</a>
   <PRE> If that doesn't help, contact Jonathan Clark at
   golgotha_source@usa.net (Subject should have "GOLG" in it)
 ***********************************************************************/
//This file belongs to the revival project only
#include "pch.h"
#include "lisp/lisp.h"
#include "lisp/li_types.h"
#include "lisp/li_init.h"
#include "init/init.h"
#include "render/r1_api.h"
//#include <stdlib.h>
//#include <stdio.h>
//#include <time.h>
#include <math.h>

li_bignum * li_bigzero;
li_object *li_read_number_in_radix(int rd,char * tk);
li_bignum *li_make_bignum(li_object * o,li_environment * env);
li_bignum *li_make_bignum(sw32 in)
{
	sw32 l=10;
	char buf[11]={
		0,0,0,0,0,0,0,0,0,0,0
	};
	char sign=in<0;

	in=abs(in);
	int i=9;
	ldiv_t ld;
	while (in>0)
	{
		ld=ldiv(in,10); //gives remainder and result in same step
		buf[i]=(w8)ld.rem;
		in=ld.quot;
		i--;
	}
	return new li_bignum(l,buf,sign);
}

li_bignum *li_make_bignum_d(double in)
{
	//Warning: This code requires sizeof(double)==sizeof(w64)
	//and 52bit mantissa / 11 bits exponent on double
	if (fabs(in)<0.5)
	{
		return li_bigzero;
	}
	//char signum=in<0;
	//in must be possitvite for log10
	double msd=log10(fabs(in));
	sw32 len=(sw32)ceil(msd);
	char * buf=new char[len+10];

	//new and easy code (not very optimized, but working)
	sprintf(buf,"%.0f",in); //print out the entire number
	char * dot=strchr(buf,'.');
	if (dot)
	{
		(*dot)=0x0;
	}                    //set a 0 where the point was

	//old and bad code (very buggy, very system-dependent, very inappropriate)
	/*w64 *dataptr=(w64*) &in;
	   w64 data=(*dataptr);//msb (the sign) is allways possitive here, so shift should suffice
	   data=data &  0xfffffffffffff;
	   data=data | 0x10000000000000;//put in the ommited leading 1
	   memset(buf,0,len);
	   sw32 start=len>=16?16:len;//cannot have more significant digits
	   sw32 sigdigits=16;//if the value is smaller than 1E16, we must
	   //drop of some digits
	   do
	   	{
	   	if (sigdigits<=start)
	   		{
	   		buf[sigdigits]=(w8)data%10;
	   		}
	   	data=data/10;
	   	sigdigits--;
	   	}while (sigdigits>=0);*/
	li_object * rs=li_read_number_in_radix(10,buf); //new li_bignum(len+1,buf,signum);
	delete [] buf;
	return li_make_bignum(rs,0);
	//get only needs the env for errors, but there shan't be any errors hereafter *hoping*

}

//converts anything to a bignum (if possible)
li_bignum *li_make_bignum(li_object * o,li_environment * env)
{
	//o=li_eval(li_car(o,env),env);
	if (o->type()==LI_BIGNUM)
	{
		return li_bignum::get(o,env);
	}
	if (o->type()==LI_INT)
	{
		return li_make_bignum(li_int::get(o,env)->value());
	}
	if (o->type()==LI_FLOAT)
	{
		return li_make_bignum_d(li_float::get(o,env)->value());
	}
	li_error(env,"USER: Calculation expects an argument of type number, you gave %O",o);
	return 0;
}

li_object *li_user_make_bignum(li_object * o,li_environment * env)
{
	return li_make_bignum(li_eval(li_car(o,env),env),env);
}

//also removes any leading zeroes.
li_bignum *li_copy_bignum(li_bignum * a, char newsign)
{
	char * abuf=a->value(),* savbuf=abuf;

	while ((*abuf)==0) abuf++;


	return new li_bignum(a->get_length()-(abuf-savbuf),abuf,newsign);
}

i4_bool li_equal_bignum(li_bignum * a, li_bignum * b)
{
	//li_bignum *c=li_copy_bignum(a,0);
	//li_bignum *d=li_copy_bignum(b,0),*temp;
	if (a->get_signum()!=b->get_signum()) //neg - pos
	{
		return i4_F;
	}



	sw32 alen=a->get_length(),blen=b->get_length(),newlen=alen>blen ? alen : blen;
	alen--;
	blen--;
	sw32 len=newlen-1;
	//w16 curval=0;//current sum (lower byte) and borrow (high byte)
	w8 aval,bval;
	char * abuf=a->value()+alen,* bbuf=b->value()+blen;
	do
	{
		aval=abuf>=a->value() ? (*(abuf--)) : 0;
		bval=bbuf>=b->value() ? (*(bbuf--)) : 0;

		if(aval!=bval)
		{
			return i4_F;
		}

		len--;
	}
	while (len>=0);
	return i4_T;

}

LI_HEADER(user_equal_bignum) {
	if (li_equal_bignum(li_make_bignum(li_eval(li_first(o,env),env),env),
						li_make_bignum(li_eval(li_second(o,env),env),env)))
	{
		return li_true_sym;
	}
	else
	{
		return li_nil;
	}
}

li_bignum *li_sub_bignum(li_bignum * a, li_bignum * b);
i4_bool li_greater_bignum(li_bignum * a, li_bignum * b)
{
	//new code (independent)
	char * abuf=a->value();
	i4_bool signinverse=i4_T;

	if (a->get_signum()&&b->get_signum())
	{
		signinverse=i4_F;
	}
	if (a->get_signum()&& !b->get_signum())
	{
		return i4_F;
	}
	if (!a->get_signum() && b->get_signum())
	{
		return i4_T;
	}
	char * bbuf=b->value();
	sw32 alen=a->get_length(),blen=b->get_length(),astart=0,bstart=0;
	while ((*abuf)==0&&alen>astart)
	{
		astart++;
		abuf++;
	}
	while ((*bbuf)==0&&blen>bstart)
	{
		bstart++;
		bbuf++;
	}
	if (alen-astart>blen-bstart)
	{
		return signinverse;
	}
	if (alen-astart<blen-bstart)
	{
		return !signinverse;
	}
	while(astart<=alen&&bstart<=blen)
	{
		if ((*abuf)>(*bbuf))
		{
			return signinverse;
		}
		if ((*abuf++)<(*bbuf++))
		{
			return !signinverse;
		}

		astart++;
		bstart++;
	}
	//if we arrive here, both are equal, so greater returns false
	return !signinverse;

	//old (tail-bitting) code
	/*li_bignum *s=li_sub_bignum(a,b);
	   if (s->get_signum())
	   	return i4_F;
	   else
	   	return i4_T;*/
}
LI_HEADER(user_greater_bignum) {
	if (li_greater_bignum(li_make_bignum(li_eval(li_first(o,env),env),env),
						  li_make_bignum(li_eval(li_second(o,env),env),env)))
	{
		return li_true_sym;
	}
	else
	{
		return li_nil;
	}
}

LI_HEADER(user_smaller_bignum) {
	if (li_greater_bignum(li_make_bignum(li_eval(li_first(o,env),env),env),
						  li_make_bignum(li_eval(li_second(o,env),env),env)))
	{
		return li_nil;
	}
	else
	{
		return li_true_sym;
	}
}

LI_HEADER(user_greaterequal_bignum) {
	li_bignum * a=li_make_bignum(li_eval(li_first(o,env),env),env);
	li_bignum * b=li_make_bignum(li_eval(li_second(o,env),env),env);

	if (li_greater_bignum(a,b)||li_equal_bignum(a,b))
	{
		return li_true_sym;
	}
	else
	{
		return li_nil;
	}
}

LI_HEADER(user_smallerequal_bignum) {
	li_bignum * a=li_make_bignum(li_eval(li_first(o,env),env),env);
	li_bignum * b=li_make_bignum(li_eval(li_second(o,env),env),env);

	if (li_greater_bignum(b,a)||li_equal_bignum(a,b))
	{
		return li_true_sym;
	}
	else
	{
		return li_nil;
	}
}


static li_bignum *li_private_mult_bignum(li_bignum * a, w8 value)
{
	if (value==10) //shift left one digit (means add an entry to the right)
	{
		li_bignum * rs=new li_bignum(a->get_length()+1,a->value(),0);
		rs->value()[a->get_length()]=0x0; //quite a hack, but saves time
		return rs;
	}
	sw32 newlen=a->get_length()+1;
	char * buf=new char[newlen];
	char * abuf=a->value()+a->get_length()-1;
	sw32 i=newlen-1;
	w16 curval=0;
	ldiv_t ld;
	memset(buf,0,newlen);
	do
	{
		curval=((abuf>=a->value() ? (*(abuf--)) : 0)*value)+curval;
		ld=ldiv(curval,10);
		buf[i]=(w8)ld.rem;
		curval=(w16)ld.quot;
		i--;
	}
	while (i>=0);
	li_bignum * r=new li_bignum(newlen,buf,0);
	delete [] buf;
	return r;
}

li_bignum *li_add_bignum(li_bignum * a,li_bignum * b); //circular recursive

li_bignum *li_sub_bignum(li_bignum * a, li_bignum * b)
{
	li_bignum * c=li_copy_bignum(a,0);
	li_bignum * d=li_copy_bignum(b,0),* temp;

	if (a->get_signum()&& !b->get_signum()) //neg - pos
	{
		return li_copy_bignum(li_add_bignum(c,d),1);
	}
	if (!a->get_signum()&& b->get_signum())
	{
		return li_copy_bignum(li_add_bignum(c,d),0);
	}
	char sig;
	if(li_greater_bignum(c,d))
	{
		sig=a->get_signum();
	}
	else
	{
		sig=b->get_signum();
		temp=a; //exchange them, since we can only subtract the smaller from the greater
		a=b;
		b=temp;
	}

	sw32 alen=a->get_length(),blen=b->get_length(),newlen=alen>blen ? alen : blen;
	alen--;
	blen--;
	char * buf=new char[newlen];
	sw32 len=newlen-1;
	w16 curval=0; //current sum (lower byte) and borrow (high byte)
	w8 aval,bval;
	char * abuf=a->value()+alen,* bbuf=b->value()+blen;
	memset(buf,0,newlen);
	do
	{
		aval=abuf>=a->value() ? (*(abuf--)) : 0;
		bval=bbuf>=b->value() ? (*(bbuf--)) : 0;
		bval+=curval;
		if (bval>aval)
		{
			curval=0x100+(aval+10-bval);
		}
		else
		{
			curval=aval-bval;
		}
		buf[len]=(w8)curval;
		curval>>=8;
		len--;
	}
	while (len>=0);
	li_bignum * rs=new li_bignum(newlen,buf,sig);
	delete[] buf;
	return rs;
}



li_object *li_bestfit(li_bignum * in)
//returns a bignum if needed, an int otherwise
{
	sw32 len=in->get_length()-1;
	sw32 res=0;

	if (len>10)
	{
		return in;
	}
	while (len)
	{
		if (res>(0x7fffffff-10))
		{
			return in;
		}
		res=res*10+in->value()[len];
		len--;
	}
	if (in->get_signum())
	{
		res= -res;
	}
	return new li_int(res);
}

class li_math_buffer_class :
	public i4_init_class
{
public:
	char * buffer;
	w32 length;
	li_math_buffer_class()
	{
		buffer=0;
		length=0;
	}
	void resize(w32 newsize)
	{
		if (newsize>length)
		{
			if (buffer)
			{
				delete[] buffer;
			}
			buffer=new char[newsize];
			length=newsize;
		}
	}
	void init()
	{
		resize(1000);
	}
	void uninit()
	{
		delete[] buffer;
		length=0;
	}
	int init_type()
	{
		return I4_INIT_TYPE_LISP_MEMORY;
	}
	void clear()
	{
		memset(buffer,0,length);
	}
} limb;   //li memory buffer;

li_bignum *li_add_bignum(li_bignum * a,li_bignum * b)
{
	if (a->get_signum()!=b->get_signum())
	{
		//do a subtraction instead;
		li_bignum * c=li_copy_bignum(a,0);
		li_bignum * d=li_copy_bignum(b,0);
		li_bignum * r=li_sub_bignum(c,d);
		if (li_greater_bignum(c,d))
		{
			return li_copy_bignum(r,a->get_signum());
		}
		else
		{
			return li_copy_bignum(r,b->get_signum());
		}
	}
	sw32 alen=a->get_length(),blen=b->get_length(),newlen=alen>blen ? alen+2 : blen+2;
	alen--;
	blen--;
	//char *buf=new char[newlen];
	if (limb.length<newlen)
	{
		limb.resize(newlen);
	}
	char * buf=limb.buffer;
	char sig=a->get_signum();
	sw32 len=newlen-1;
	w8 curval=0; //current sum (lower byte) and carry (if larger 10)
	sw32 firstused=len;
	w8 aval,bval;
//	ldiv_t ld;
	char * abuf=a->value()+alen,* bbuf=b->value()+blen;
	memset(buf,0,newlen); //potentially faster than clear();
	do
	{
		aval=abuf>=a->value() ? (*(abuf--)) : 0;
		bval=bbuf>=b->value() ? (*(bbuf--)) : 0;
		curval=aval+bval+curval;
		if (curval)
		{
			firstused=len;
		}
		//ld=ldiv(curval,10);
		//buf[len]=(w8)ld.rem;
		//curval=(w16)ld.quot;
		if (curval>=10)
		{
			buf[len]=(curval-10);
			curval=1;
			len--; //put here as hint for optimizer.
		}
		else
		{
			buf[len]=curval;
			curval=0;
			len--;
		}
	}
	while (len>=0);
	li_bignum * rs=new li_bignum(newlen-firstused,buf+firstused,sig);
	//delete [] buf;
	return rs;
}

li_bignum *li_mult_bignum(li_bignum * a, li_bignum * b)
{
	char sig;

	if (a->get_signum()==b->get_signum())
	{
		sig=0;
	}
	else
	{
		sig=1;
	}
	sw32 alen=a->get_length(),blen=b->get_length(),
		 newlen=alen+blen+2,len=newlen-1;
	alen--;
	blen--;
	char * abuf=a->value()+alen;
	//char *buf=new char[newlen];
	li_bignum * c=li_copy_bignum(b,0),* d,* rs;
	w16 curval=0;
	w8 aval;
	rs=li_bigzero;
	//memset(buf,0,newlen);
	do
	{
		aval=abuf>=a->value() ? (*(abuf--)) : 0;
		d=li_private_mult_bignum(c,aval);
		c=li_private_mult_bignum(c,10); //shift left one position
		rs=li_add_bignum(rs,d);
		len--;
	}
	while (len>=0);
	//li_bignum *rs=new li_bignum(newlen,buf,sig);
	//delete [] buf;
	//return rs;
	return li_copy_bignum(rs,sig);
}

li_bignum *li_add_bignum(li_bignum * a, int b)
{
	return li_add_bignum(a,li_make_bignum(b));
}

li_bignum *li_sub_bignum(li_bignum * a, int b)
{
	return li_sub_bignum(a,li_make_bignum(b));
}

li_bignum *li_mult_bignum(li_bignum * a, int b)
{
	return li_mult_bignum(a,li_make_bignum(b));
}

/** Attempt to divide a by b */
li_bignum *li_div_bignum(li_bignum * a, li_bignum * b)
{

	if (li_equal_bignum(a,li_bigzero))
	{
		li_error(0,"USER: Encountered a divide by zero attempt.");
		return 0;
	}
	if (li_greater_bignum(b,a))
	{
		return li_bigzero;
	}
	if (li_equal_bignum(a,b))
	{
		return li_make_bignum(1);
	}
	li_bignum * a1=li_copy_bignum(a,0);
	li_bignum * b1=li_copy_bignum(b,0);
	w32 lena1=a1->get_length();
	w32 lenb1=b1->get_length();
	li_bignum * upperguess=0;
	li_bignum * lowerguess=0;
	li_bignum * actresult=0;
	//try to do a upper length guess on the result size
	w32 lguess=(lena1-lenb1)+2;
	li_bignum * guesslength=0;
	char * num=new char[lguess+1];
	memset(num,9,lguess);
	upperguess=new li_bignum(lguess,num,0);

	char currentdigit=0;
	w32 curpoint=0;
	actresult=li_mult_bignum(upperguess,b1);
	if (li_greater_bignum(a,actresult))
	{
		li_error(0,"INTERNAL: Unexspected behaviour of division result");
		return 0;
	}
	num[curpoint]=0;
	lowerguess=new li_bignum(lguess,num,0);
	actresult=li_mult_bignum(lowerguess,b1);
	while (li_greater_bignum(actresult,a1))
	{
		// it's not a lower guess then.
		upperguess=lowerguess;
		curpoint++;
		if (curpoint>lguess)
		{
			li_error(0,"INTERNAL: Could not find lower estimate for division result");
			delete[] num;
			return 0;
		}
		num[curpoint]=0;
		lowerguess=new li_bignum(lguess,num,0);
		actresult=li_mult_bignum(lowerguess,b1);
	}
	//we now have an upper and a lower guess on the result.
	//Continuation as follows: Do a binary search on the leftmost digit
	//until upper and lower bound on that digit differ by 1. Then
	//switch to the next digit and so forth.
	w32 pos=0;
	char adigit=0,olddigit;
	li_bignum * oldupper;
	do
	{
		actresult=li_mult_bignum(upperguess,b1);
		do
		{

			//pos=-1;
			//do
			//	{
			//	pos++;
			//	adigit=upperguess->value()[pos];
			//	}while (adigit==0);
			adigit=upperguess->value()[pos];
			olddigit=adigit;
			//upperguess must not change its size (independent of its value)
			oldupper=new li_bignum(upperguess->get_length(),
								   upperguess->value(),0); //li_copy_bignum(upperguess,0);
			if (adigit>0)
			{
				adigit--;
				upperguess->value()[pos]=adigit;
			}
			else
			{
				upperguess->value()[pos]=0;
				pos++;
				upperguess->value()[pos]--; //was 9 and that was to big.
			}
			actresult=li_mult_bignum(upperguess,b1);
			if (li_equal_bignum(actresult,a1))
			{
				delete[] num;
				return li_copy_bignum(upperguess,a->get_signum()==b->get_signum() ? 0 : 1);
			}
		}
		while(li_greater_bignum(actresult,a1));
		lowerguess=upperguess;
		upperguess=oldupper;
		pos++;
	}
	while(pos<upperguess->get_length());
	delete[] num;
	return li_copy_bignum(upperguess,a->get_signum()==b->get_signum() ? 0 : 1);
}

li_object *li_add(li_object * o,li_environment * env)
{
	li_object * o1=li_eval(li_first(o,env),env);

	if (!o1)
	{
		return new li_int(0);
	}
	if (o1->type()==LI_INT /*||o1->type()==LI_BIGNUM*/)
	{
		/*li_bignum *summe,*summand;
		   if (o1->type()==LI_INT)
		   	summe=li_make_bignum(li_int::get(o1,env)->value());
		   	else
		   	summe=li_bignum::get(o1,env);
		   li_object *o2=li_cdr(o,env);
		   while(o2)
		   	{
		   	if (o2->type()==LI_INT)
		   		summand=li_make_bignum(li_int::get(o2,env)->value());
		   	else
		   		summand=li_bignum::get(o2,env);
		   	summe=li_add_bignum(summe,summand);
		   	o2=li_cdr(o2,env);
		   	}
		   return li_bestfit(summe);*/
		int summe=li_get_int(o1,env);
		li_object * o2=li_cdr(o,env);
		while (o2)
		{
			summe+=li_get_int(li_eval(li_car(o2,env),env),env);
			o2=li_cdr(o2,env);
		}
		return new li_int(summe);
	}
	else
	{
		double summe=li_get_float(o1,env);
		li_object * o3=li_cdr(o,env);
		while(o3)
		{
			summe+=li_get_float(li_eval(li_car(o3,env),env),env);
			o3=li_cdr(o3,env);
		}
		return new li_float(summe);
	}
}


//same, but works with bigints
li_object *li_add_b(li_object * o,li_environment * env)
{
	li_object * o1=li_eval(li_first(o,env),env);

	if (!o1)
	{
		return li_bigzero;
	}
	li_bignum * summe,* summand;
	summe=(li_bignum *)li_make_bignum(o1,env);
	li_object * o2=li_cdr(o,env);
	while(o2)
	{
		summand=li_make_bignum(li_eval(li_car(o2,env),env),env);
		summe=li_add_bignum(summe,summand);
		o2=li_cdr(o2,env);
	}
	return summe;
}

li_object *li_subtract(li_object * o,li_environment * env)
{
	li_object * o1=li_eval(li_first(o,env),env);

	if (o1->type()==LI_INT)
	{
		int summe=li_get_int(o1,env);
		li_object * o2=li_cdr(o,env);
		if (!o2)
		{
			return new li_int(-summe);
		}                                  // - with one arg means complement
		while (o2)
		{
			summe-=li_get_int(li_eval(li_car(o2,env),env),env);
			o2=li_cdr(o2,env);
		}
		return new li_int(summe);
	}
	else
	{
		double summe=li_get_float(o1,env);
		li_object * o3=li_cdr(o,env);
		if (!o3)
		{
			return new li_float(-summe);
		}
		while(o3)
		{
			summe-=li_get_float(li_eval(li_car(o3,env),env),env);
			o3=li_cdr(o3,env);
		}
		return new li_float(summe);
	}
}

LI_HEADER(sub_b) {
	li_object * o1=li_eval(li_first(o,env),env);

	if (!o1)
	{
		return li_bigzero;
	}
	li_bignum * dif,* subtract;
	dif=li_make_bignum(o1,env);
	li_object * o2=li_cdr(o,env);
	while(o2)
	{
		subtract=li_make_bignum(li_eval(li_car(o2,env),env),env);
		dif=li_sub_bignum(dif,subtract);
		o2=li_cdr(o2,env);
	}
	return dif;
}


li_object *li_multiply(li_object * o,li_environment * env)
{
	li_object * o1=li_eval(li_first(o,env),env);

	if (!o1)
	{
		return new li_int(1);
	}
	if (o1->type()==LI_INT)
	{
		int summe=li_get_int(o1,env);
		li_object * o2=li_cdr(o,env);
		while (o2)
		{
			summe*=li_get_int(li_eval(li_car(o2,env),env),env);
			o2=li_cdr(o2,env);
		}
		return new li_int(summe);
	}
	else
	{
		double summe=li_get_float(o1,env);
		li_object * o3=li_cdr(o,env);
		while(o3)
		{
			summe*=li_get_float(li_eval(li_car(o3,env),env),env);
			o3=li_cdr(o3,env);
		}
		return new li_float(summe);
	}
}

LI_HEADER(mult_b) {

	li_bignum * o1=li_make_bignum(li_eval(li_first(o,env),env),env);

	if (!o1)
	{
		return new li_int(1);
	}
	li_bignum * summe=o1,* mult;
	li_object * o2=li_cdr(o,env);
	while (o2)
	{
		mult=li_make_bignum(li_eval(li_car(o2,env),env),env);
		summe=li_mult_bignum(summe,mult);
		o2=li_cdr(o2,env);
	}
	return summe;


}


li_object *li_divide(li_object * o,li_environment * env)
{
	li_object * o1=li_eval(li_first(o,env),env);

	if (o1->type()==LI_INT)
	{
		int summe=li_get_int(o1,env);
		li_object * o2=li_cdr(o,env);
		while (o2)
		{
			int divid=li_get_int(li_eval(li_car(o2,env),env),env);
			if (divid==0)
			{
				li_error(env,"USER: Division by zero encountered in %O\n",o);
				summe=0;
			}
			else
			{
				summe/=divid;
			}
			o2=li_cdr(o2,env);
		}
		return new li_int(summe);
	}
	else
	{
		double summe=li_get_float(o1,env);
		li_object * o3=li_cdr(o,env);
		while(o3)
		{
			double divid=li_get_float(li_eval(li_car(o3,env),env),env);
			if (divid==0)
			{
				li_error(env,"USER: Division by zero encountered in %O\n",o);
				summe=0;
			}
			else
			{
				summe/=divid;
			}

			o3=li_cdr(o3,env);
		}
		return new li_float(summe);
	}
}

LI_HEADER(divide_b) {
	li_bignum * o1=li_make_bignum(li_eval(li_first(o,env),env),env);

	if (!o1)
	{
		return new li_int(1);
	}
	li_bignum * quotient=o1,* dividend;
	li_object * o2=li_cdr(o,env);
	while (o2)
	{
		dividend=li_make_bignum(li_eval(li_car(o2,env),env),env);
		quotient=li_div_bignum(quotient,dividend);
		o2=li_cdr(o2,env);
	}
	return quotient;

}


LI_HEADER(exp) {
	double d=li_get_float(li_eval(li_first(o,env),env),env);

	return new li_float(exp(d));
}

LI_HEADER(pow) {
	double x=li_get_float(li_eval(li_first(o,env),env),env);
	double y=li_get_float(li_eval(li_second(o,env),env),env);

	return new li_float(pow(x,y));
}

LI_HEADER(tan) {
	double d=li_get_float(li_eval(li_first(o,env),env),env);

	return new li_float(tan(d));
}

LI_HEADER(cot) {
	double d=li_get_float(li_eval(li_first(o,env),env),env);

	return new li_float(1/tan(d));
}

LI_HEADER(log) {
	double d=li_get_float(li_eval(li_first(o,env),env),env);

	return new li_float(log(d));
}

LI_HEADER(atan) {
	double d=li_get_float(li_eval(li_first(o,env),env),env);

	return new li_float(atan(d));
}

LI_HEADER(acos) {
	double d=li_get_float(li_eval(li_first(o,env),env),env);

	return new li_float(acos(d));
}

LI_HEADER(asin) {
	double d=li_get_float(li_eval(li_first(o,env),env),env);

	return new li_float(tan(d));
}
LI_HEADER(sqr) {
	double d=li_get_float(li_eval(li_first(o,env),env),env);

	return new li_float(d*d);
}
LI_HEADER(sqrt) {
	double d=li_get_float(li_eval(li_first(o,env),env),env);

	return new li_float(sqrt(d));
}

LI_HEADER(atan2) {
	double y=li_get_float(li_eval(li_first(o,env),env),env);
	double x=li_get_float(li_eval(li_second(o,env),env),env);

	return new li_float(atan2(y,x));
}

LI_HEADER(integerp) {
	li_object * at=li_eval(li_car(o,env),env);

	if (at==0||at==li_nil)
	{
		return li_nil;
	}
	int t=at->type();
	if (t==LI_INT||t==LI_BIGNUM)
	{
		return li_true_sym;
	}
	else
	{
		return li_nil;
	}
}

LI_HEADER(realp) {
	li_object * at=li_eval(li_car(o,env),env);

	if (at==0||at==li_nil)
	{
		return li_nil;
	}
	int t=at->type();
	if (t==LI_FLOAT)
	{
		return li_true_sym;
	}
	else
	{
		return li_nil;
	}
}

LI_HEADER(characterp) {
	li_object * at=li_eval(li_car(o,env),env);

	if (at==0||at==li_nil)
	{
		return li_nil;
	}
	int t=at->type();
	if (t==LI_CHARACTER)
	{
		return li_true_sym;
	}
	else
	{
		return li_nil;
	}
}
LI_HEADER(stringp) {
	li_object * at=li_eval(li_car(o,env),env);

	if (at==0||at==li_nil)
	{
		return li_nil;
	}
	int t=at->type();
	if (t==LI_STRING)
	{
		return li_true_sym;
	}
	else
	{
		return li_nil;
	}
}

static long rand_seed=4711;

/*LI_HEADER(srand)
   	{
   	rand_seed=(long)li_get_int(li_eval(li_car(o,env),env),env);
   	return li_nil;
   	}
 */

#define rand_size 1024
#define rand_mask (rand_size-1)
long rand_table[rand_size];
long _rnd1=193;
long _rnd2=0;
long _rnd3=122231;

LI_HEADER(random) {
	long value=0;
	long dx=_rnd3&rand_mask,pdx;
	long ax=_rnd1;

	ax=dx*ax;

	_rnd2++;
	dx=_rnd2&rand_mask;
	pdx=(_rnd2<<10)&rand_mask;
	_rnd3+=3;
	ax=ax+rand_table[pdx];
	rand_table[pdx]+=ax;
	ax=rand_table[dx];
	_rnd1=ax;
	value=ax&0x7fffffff; //kill of sign bit.


	return new li_int(value%li_get_int(li_eval(li_car(o,env),env),env));
}



li_object *li_read_number_in_radix(int rd,char * tk)
{
	w32 actval=0,curval=0;
	char sig=1,* stk=tk;

	if (*tk=='-')
	{
		tk++;
		sig=-1;
	}
	else if (*tk=='+')
	{
		tk++;
	}
	;
	//w32 l=strlen(tk);
	w32 overflowat=2147483648/rd;
	li_bignum * b=0;
	do
	{
		if (actval>overflowat)
		{
			//need bignum, as this would overflow on next step.
			b=li_make_bignum(actval);
			actval=0;
		}
		if (b)
		{
			if (rd<=10)
			{
				b=li_private_mult_bignum(b,rd);
			}
			else
			{
				b=li_mult_bignum(b,rd);
			}
		}
		else
		{
			actval*=rd;
		}
		if (*tk>='0'&&(*tk)<='9')
		{
			curval=*tk-'0';
		}
		else
		if ((*tk)>='A'&&(*tk)<='Z')
		{
			curval=*tk-'A'+10;
		}
		else
		if ((*tk)>='a'&&(*tk)<='z')
		{
			curval=*tk-'a'+10;
		}
		else
		{
			li_error(0,"USER: Invalid character or digit in number %s for radix %d.",tk,rd);
			return new li_int(0);
		}
		if (curval>=(w32)rd)
		{
			li_error(0,"USER: Digit %d not allowed in radix %d",curval,rd);
			return new li_int(0);
		}
		if (b)
		{
			b=li_add_bignum(b,curval);
		}
		else
		{
			actval+=curval;
		}
		tk++;
	}
	while(*tk);
	if (b)
	{
		return li_copy_bignum(b,!sig);
	}                                 //uses inverse value for sig
	return new li_int((sw32)actval*sig);
}

LI_HEADER(boundp) {
	if (li_get_value(li_symbol::get(li_eval(li_car(o,env),env),env),env),env)
	{
		return li_true_sym;
	}
	else
	{
		return li_nil;
	}
}

LI_HEADER(fboundp) {
	if (li_get_fun(li_symbol::get(li_eval(li_car(o,env),env),env),env),env)
	{
		return li_true_sym;
	}
	else
	{
		return li_nil;
	}
}

LI_HEADER(setfog) {
	if (!r1_render_api_class_instance)
	{
		return 0;
	}
	r1_render_api_class_instance->set_fogging_mode(li_get_int(li_first(o,env),env),
												   (float)li_get_float(li_second(o,env),env),
												   (float)li_get_float(li_third(o,env),env));
	return li_true_sym;
};

class li_math_register_class :
	public i4_init_class
{
	virtual void init()
	{
		//create the zero object (used for performance reasons)
		li_set_value(li_get_symbol("big_zero"),li_bigzero=li_make_bignum(0));
		li_add_function("+",li_add);
		li_add_function("+b",li_add_b);
		li_add_function("-",li_subtract);
		li_add_function("-b",li_sub_b);
		li_add_function("=b",li_user_equal_bignum);
		li_add_function(">b",li_user_greater_bignum);
		li_add_function("*",li_multiply);
		li_add_function("<b",li_user_smaller_bignum);
		li_add_function(">=b",li_user_greaterequal_bignum);
		li_add_function("<=b",li_user_smallerequal_bignum);
		li_add_function("*b",li_mult_b);
		li_add_function("/",li_divide);
		li_add_function("/b",li_divide_b);
		li_add_function("exp",li_exp);
		li_add_function("pow",li_pow);
		li_add_function("sqr",li_sqr); //sin and cos are implemented
		li_add_function("sqrt",li_sqrt); //in lisp__functions
		li_add_function("atan2",li_atan2);
		li_add_function("tan",li_tan);
		li_add_function("cot",li_cot);
		li_add_function("asin",li_asin);
		li_add_function("acos",li_acos);
		li_add_function("atan",li_atan);
		li_add_function("log",li_log);
		li_add_function("make-bignum",li_user_make_bignum);
		li_add_function("random",li_random);
		li_add_function("integerp",li_integerp);
		li_add_function("realp",li_realp);
		li_add_function("characterp",li_characterp);
		li_add_function("stringp",li_stringp);
		li_add_function("boundp",li_boundp);
		li_add_function("fboundp",li_fboundp);
		li_add_function("setfog",li_setfog);
		srand( (unsigned)time( NULL ) );
		for(int i=0; i<rand_size; i++)
		{
			rand_table[i]=(((rand()<<12)+rand())<<12)+rand();
		}
	}
	virtual int init_type()
	{
		return I4_INIT_TYPE_LISP_FUNCTIONS;
	}
} li_math_register_class_instance;

#include <RcppArmadillo.h>
// [[Rcpp::depends(RcppArmadillo)]]

using namespace Rcpp;

// Helper functions for IRLS

// [[Rcpp::export]]
arma::vec initial_mu(const char* family, arma::vec y, int N) {
  
  const char* bin = "binomial";
  const char* pois = "poisson";
  const char* gaus = "gaussian";
  const char* Gamma = "Gamma";
  
  int i;
  
  arma::vec mu(N);
  
  if(std::strcmp(family, bin) == 0){
    for (i=0; i<N; i++) {
      if (y[i] < 0) {
        stop("negative values not allowed for the Binomial family");
      }
      if (y[i] > 1.0) {
        stop("# of success is larger than 1");
      }
      mu[i] = (y[i] + 0.5)/(1.0+1.0);
    }
  }else if(std::strcmp(family, pois) == 0){
    for (i=0; i<N; i++) {
      if (y[i] < 0) {
        stop("negative values not allowed for the Poisson family");
      }      
      mu[i] = y[i] + 0.1;
    }
  }else if(std::strcmp(family, gaus) == 0){
    for (i=0; i<N; i++) {
      mu[i] = y[i];
    }
  }else if(std::strcmp(family, Gamma) == 0){
    for (i=0; i<N; i++) {
      if (y[i] <= 0) {
        stop("non-poistive values not allowed for the Gamma family");
      }      
      mu[i] = y[i] + 0.1;
    }
  }
  
  return mu;
}

// [[Rcpp::export]]
arma::vec muvalid(const char* family, arma::vec mu) {
  
  const char* bin = "binomial";
  const char* pois = "poisson";
  const char* gaus = "gaussian";
  const char* Gamma = "Gamma";
  
  double minb = 0.0001; // minimum allowed binomial mu value
  double maxb = 0.9999; // maximum allowed binomial mu value
  double minp = 0.0001; // minimum allowed poisson mu value
  double gammaMin = 0.001; // miminum allowed gamma mu value
  
  int i = 0;
  int N = mu.n_elem;
  arma::vec valid(N);
  
  if(std::strcmp(family, bin) == 0){
    for(i=0; i<N; i++){
      valid(i) = (mu(i) > minb && mu(i) < maxb);
    }
  }else if(std::strcmp(family, pois) == 0){
    for(i=0; i<N; i++){
      valid(i) = (mu(i) > minp);
    }
  }else if(std::strcmp(family, gaus) == 0){
    for(i=0; i<N; i++){
      valid(i) = 1;
    }
  }else if(std::strcmp(family, Gamma) == 0){
    for(i=0; i<N; i++){
      valid(i) = (mu(i) > gammaMin);
    }
  }else{
    stop("invalid family \n");
  }
  
  return(valid);
  
}

// [[Rcpp::export]]
arma::vec mu_adjust(const char* family, arma::vec mu) {
  
  const char* bin = "binomial";
  const char* pois = "poisson";
  const char* gaus = "gaussian";
  const char* Gamma = "Gamma";
  
  double minb = 0.001; // minimum allowed binomial mu value
  double maxb = 0.999; // maximum allowed binomial mu value
  double minp = 0.001; // minimum allowed poisson mu value
  double gammaMin = 0.001; // miminum allowed gamma mu value
  
  int i = 0;
  int N = mu.n_elem;
  arma::vec mu_new = mu;
  
  if(std::strcmp(family, bin) == 0){
    for(i=0; i<N; i++){
      if(mu(i) < minb){
        mu_new(i) = minb;
      }else if(mu(i) > maxb){
        mu_new(i) = maxb;
      }
    }
  }else if(std::strcmp(family, pois) == 0){
    for(i=0; i<N; i++){
      if(mu(i) < minp){
        mu_new(i) = minp;
      }
    }
  }else if(std::strcmp(family, Gamma) == 0){
    for(i=0; i<N; i++){
      if(mu(i) < gammaMin){
        mu_new(i) = gammaMin;
      }
    }
  }else if(std::strcmp(family, gaus) == 0){ 
    // No invalid mu
    mu_new = mu;
  }else{
    stop("invalid family \n");
  }
  
  // Note: gaussian family does not have any invalid mu
  
  return(mu_new);
  
}

// [[Rcpp::export]]
arma::vec dlink(int link, arma::vec mu){
  
  int N = mu.n_elem;
  arma::vec out(N);
  arma::vec ones_vec = out.ones();
  arma::vec zeros_vec = out.zeros();
  
  switch(link){
  case 10: return(ones_vec / (mu % (ones_vec - mu))); // logit
  case 11: return(zeros_vec); // probit not yet available
  case 12: return(ones_vec/(log(ones_vec-mu)%(ones_vec-mu))); // cloglog
  case 20: return(ones_vec/mu); // log
  case 30: return(ones_vec); // identity
  case 40: return(zeros_vec - ones_vec/(mu%mu)); // inverse
  default: return(zeros_vec); 
  }
  
}

// [[Rcpp::export]]
arma::vec linkfun(int link, arma::vec mu){
  
  int N = mu.n_elem;
  arma::vec out(N);
  arma::vec ones_vec = out.ones();
  
  switch(link){
  case 10: return(log(mu/(ones_vec-mu))); // logit
  case 11: return(out.zeros()); // probit not yet available
  case 12: return(log(-ones_vec % log(ones_vec-mu))); // cloglog
  case 20: return(log(mu)); // log
  case 30: return(mu); // identity
  case 40: return(ones_vec / mu); // inverse
  default: return(out.zeros());
  }
  
}

// [[Rcpp::export]]
arma::vec invlink(int link, arma::vec eta){
  
  int N = eta.n_elem;
  arma::vec empty(N);
  arma::vec ones_vec = empty.ones();
  arma::vec zeros_vec = empty.zeros();
  
  switch(link){
  case 10: return(exp(eta) / (ones_vec + exp(eta))); // logit
  case 11: return(zeros_vec); // probit not yet available
  case 12: return(ones_vec - exp(zeros_vec - ones_vec % exp(eta))); // cloglog
  case 20: return(exp(eta)); // log
  case 30: return(eta); // identity
  case 40: return(zeros_vec - ones_vec / eta); // inverse
  default: return(zeros_vec);
  }
  
}

arma::vec varfun(const char* family, arma::vec mu){ // double phi
  
  const char* bin = "binomial";
  const char* pois = "poisson";
  const char* gaus = "gaussian";
  const char* Gamma = "Gamma";
  // const char* NB = "Negative Binomial(4)"; // MASS negative.binomial() family function
  
  int N = mu.n_elem;
  arma::vec V(N);
  arma::vec empty(N);
  arma::vec ones_vec = empty.ones();
  
  if(std::strcmp(family, bin) == 0){
    V = mu%(ones_vec-mu);
  }else if(std::strcmp(family, pois) == 0){
    V = mu;
  }else if(std::strcmp(family, gaus) == 0){
    V = ones_vec;
  }else if(std::strcmp(family, Gamma) == 0){
    V = mu%mu;
  }else{
    stop("invalid family \n");
  }
  
  // else if(std::strcmp(family, NB) == 0){
  //   V = mu + mu*mu*phi;
  // }
  
  return(V);
  
}


////////////////////////////////////////////////////////////////////////////////////////////////
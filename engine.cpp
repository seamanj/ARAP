//#include "engine.h"
//#include "uisettings.h"
//#include <QDebug>

//#define TICKS 10

//Engine::Engine()
//    : m_time(0.f),
//      m_running(false),
//      m_paused(false),
//      m_busy(false)
//{
//    assert( connect(&m_ticker, SIGNAL(timeout()), this, SLOT(update())) );
//}
//Engine::~Engine()
//{
//    if( m_running ) stop();
//}
//bool Engine::isRunning()
//{
//    return m_running;
//}

//void Engine::addParticleSystem(const shared_ptr<ParticleSystem> &spParticleSystem)
//{
//    m_spParticleSystems.push_back(spParticleSystem);
//}


//void Engine::update()
//{
//    if ( !m_busy && m_running && !m_paused ) {

//        m_busy = true;
//        for(int i = 0; i < m_spParticleSystems.size(); ++i)
//        {
//            shared_ptr<ParticleSystem>& spParticleSystem = m_spParticleSystems[i];
            
////            spParticleSystem->m_currentVelocities = spParticleSystem->m_currentVelocities + UiSettings::timeStep()  * spParticleSystem->m_externalForce;//* spParticleSystem->m_invMassMatrix
////            spParticleSystem->m_currentPositions = spParticleSystem->m_currentPositions + UiSettings::timeStep() * spParticleSystem->m_currentVelocities;

//            QMap<int, shared_ptr<Particle>>& particles = spParticleSystem->getParticles();
//            int size = particles.size();
//            for(int j = 0; j < size; ++j)
//            {
//                shared_ptr<Particle> spParticle = particles[j];
//                if( !spParticle->m_dragged )
//                {
//                    spParticleSystem->m_currentVelocities.block_vector(j) = spParticleSystem->m_currentVelocities.block_vector(j) + UiSettings::timeStep()  * spParticleSystem->m_externalForce.block_vector(j);//* spParticleSystem->m_invMassMatrix
//                    spParticleSystem->m_currentPositions.block_vector(j) = spParticleSystem->m_currentPositions.block_vector(j) + UiSettings::timeStep() * spParticleSystem->m_currentVelocities.block_vector(j);
//                }
//            }
            
//            //collide
//            size = particles.size();
//            for(int j = 0; j < size; ++j)
//            {
                
//                shared_ptr<Particle> spParticle = particles[j];
//                if( spParticle->m_dragged )
//                    continue;
                
//                float bounder = 6.2;
//                if( spParticleSystem->m_currentPositions.block_vector(j).y() < 0 )
//                    spParticleSystem->m_currentPositions.block_vector(j).y() = 0;
//                if( spParticleSystem->m_currentPositions.block_vector(j).x() > bounder )
//                    spParticleSystem->m_currentPositions.block_vector(j).x() = bounder;
//                if( spParticleSystem->m_currentPositions.block_vector(j).x() < -bounder )
//                    spParticleSystem->m_currentPositions.block_vector(j).x() = -bounder;
//                if( spParticleSystem->m_currentPositions.block_vector(j).z() > bounder )
//                    spParticleSystem->m_currentPositions.block_vector(j).z() = bounder;
//                if( spParticleSystem->m_currentPositions.block_vector(j).z() < -bounder )
//                    spParticleSystem->m_currentPositions.block_vector(j).z() = -bounder;
//            }
            
//            //cal t0
//            size = particles.size();
//            for(int j = 0; j < size; ++j)
//            {
//                shared_ptr<Particle> spParticle = particles[j];
//                if( spParticle->m_dragged )
//                {
//                    spParticleSystem->m_massMatrix.coeffRef(j * 3, j * 3) = 100;
//                    spParticleSystem->m_massMatrix.coeffRef(j * 3 + 1, j * 3 + 1) =  100;
//                    spParticleSystem->m_massMatrix.coeffRef(j * 3 + 2, j * 3 + 2) =  100;
//                }
//                else
//                {
//                    spParticleSystem->m_massMatrix.coeffRef(j * 3, j * 3) = 1;
//                    spParticleSystem->m_massMatrix.coeffRef(j * 3 + 1, j * 3 + 1) = 1;
//                    spParticleSystem->m_massMatrix.coeffRef(j * 3 + 2, j * 3 + 2) = 1;
//                }
//            }
            
//            VectorX mx0;
//            mx0.resize( spParticleSystem->m_verticesNum * 3);
//            mx0 = spParticleSystem->m_massMatrix * spParticleSystem->m_originalPositions;
//            EigenVector3 sumMX0(0,0,0);
//            size = mx0.rows();
//            for( int i = 0; i < size / 3; ++i)
//            {
//                sumMX0 += mx0.block_vector(i);
//            }
//            float sumM = 0.0f;
//            int outerSize = spParticleSystem->m_massMatrix.outerSize();
//            for( int i = 0; i < outerSize; i += 3)
//            {
//                sumM += spParticleSystem->m_massMatrix.coeff(i, i);
//            }
//            spParticleSystem->m_t0 = sumMX0 / sumM;
            
//            spParticleSystem->m_Aqq.setZero();
//            if( spParticleSystem->m_type == ParticleSystem::Quadratic )
//                spParticleSystem->m_AqqTilda.setZero();
//            size =  spParticleSystem->m_q.rows();
//            for( int i = 0, j = 0; i < size / 3; ++i, j += 3)
//            {
//                EigenVector3 q = spParticleSystem->m_originalPositions.block_vector(i) -  spParticleSystem->m_t0;
//                spParticleSystem->m_q.block_vector(i) = q;
//                spParticleSystem->m_Aqq +=  spParticleSystem->m_q.block_vector(i) *  spParticleSystem->m_q.block_vector(i).transpose() *  spParticleSystem->m_massMatrix.block(j, j, 3, 3);
//                if( spParticleSystem->m_type == ParticleSystem::Quadratic )
//                {
//                    spParticleSystem->m_qTilda.block_vector9x1(i) << q, q.x() * q.x(), q.y() * q.y(), q.z() * q.z(), q.x() * q.y(), q.y() * q.z(), q.z() * q.x();
//                    spParticleSystem->m_AqqTilda += spParticleSystem->m_qTilda.block_vector9x1(i) *  spParticleSystem->m_qTilda.block_vector9x1(i).transpose() *  spParticleSystem->m_massMatrix.coeff(j, j);
//                }
//            }
            
//            // cal t,p,A
//            VectorX mx;
//            mx.resize( spParticleSystem->m_verticesNum * 3);
//            mx = spParticleSystem->m_massMatrix * spParticleSystem->m_currentPositions;
           
//            EigenVector3 sumMX(0,0,0);
//            size = mx.rows();
//            for( int i = 0; i < size / 3; ++i)
//            {
//                sumMX += mx.block_vector(i);
                
//            }
//            spParticleSystem->m_t = sumMX / sumM;

            
            
//            spParticleSystem->m_Apq.setZero();
//            if( spParticleSystem->m_type == ParticleSystem::Quadratic )
//                spParticleSystem->m_ApqTilda.setZero();
//            size = spParticleSystem->m_p.rows();
//            for( int i = 0, j = 0; i < size / 3; ++i, j += 3)
//            {
//                spParticleSystem->m_p.block_vector(i) = spParticleSystem->m_currentPositions.block_vector(i) - spParticleSystem->m_t;
//                spParticleSystem->m_Apq += spParticleSystem->m_p.block_vector(i) * spParticleSystem->m_q.block_vector(i).transpose() * spParticleSystem->m_massMatrix.block(j, j, 3, 3);
//                if( spParticleSystem->m_type == ParticleSystem::Quadratic )
//                {
//                    spParticleSystem->m_ApqTilda += spParticleSystem->m_p.block_vector(i) *  spParticleSystem->m_qTilda.block_vector9x1(i).transpose() *  spParticleSystem->m_massMatrix.coeff(j, j);
//                }
//            }
            
            
//            spParticleSystem->m_A.setZero();
//            spParticleSystem->m_A = spParticleSystem->m_Apq * spParticleSystem->m_Aqq.inverse();
//            if( spParticleSystem->m_type == ParticleSystem::Quadratic )
//            {
//                spParticleSystem->m_ATilda.setZero();
//                spParticleSystem->m_ATilda = spParticleSystem->m_ApqTilda * spParticleSystem->m_AqqTilda.inverse();
//            }
            
//            if( spParticleSystem->m_type == ParticleSystem::Linear )
//            {
//                float det = spParticleSystem->m_A.determinant();
//                if( det != 0.0f)
//                {
//                    det = 1.0f / pow(fabs(det), 1/3.f);
//                    if( det > 3.0f ) det  = 3.0f;
//                    spParticleSystem->m_A *= det;
//                }
//            }
            
            
            
//            Eigen::JacobiSVD<Eigen::MatrixXf> svd(spParticleSystem->m_A, Eigen::ComputeThinU | Eigen::ComputeThinV);
//            spParticleSystem->m_R = svd.matrixU() * svd.matrixV().transpose();
            
//            if( spParticleSystem->m_type == ParticleSystem::Quadratic )
//            {
//                Eigen::JacobiSVD<Eigen::MatrixXf> svdTilda(spParticleSystem->m_ATilda.block(0, 0, 3, 3), Eigen::ComputeThinU | Eigen::ComputeThinV);
//                spParticleSystem->m_RTilda << svdTilda.matrixU() * svdTilda.matrixV().transpose(), Eigen::MatrixXf::Zero(3,6) ;
//            }
            
////            size = spParticleSystem->m_g.rows();
////            for(int i = 0; i < size / 3; ++i )
////            {
////                spParticleSystem->m_g.block_vector(i) = spParticleSystem->m_R * spParticleSystem->m_q.block_vector(i) + spParticleSystem->m_t;
////            }
//            size = particles.size();
//            for(int j = 0; j < size; ++j)
//            {
//                shared_ptr<Particle> spParticle = particles[j];
//                if( spParticle->m_dragged )
//                    spParticleSystem->m_g.block_vector(j) = spParticleSystem->m_currentPositions.block_vector(j);
//                else
//                {
//                    if( spParticleSystem->m_type == ParticleSystem::Rigid )
//                        spParticleSystem->m_g.block_vector(j) = spParticleSystem->m_R * spParticleSystem->m_q.block_vector(j) + spParticleSystem->m_t;
//                    else if(spParticleSystem->m_type == ParticleSystem::Linear)
//                    {
//                        spParticleSystem->m_g.block_vector(j) = (UiSettings::beta() * spParticleSystem->m_R + (1 - UiSettings::beta()) * spParticleSystem->m_A) * spParticleSystem->m_q.block_vector(j) + spParticleSystem->m_t;
//                    }
//                    else if(spParticleSystem->m_type == ParticleSystem::Quadratic)
//                    {
//                        spParticleSystem->m_g.block_vector(j) = (UiSettings::beta() * spParticleSystem->m_RTilda + (1 - UiSettings::beta()) * spParticleSystem->m_ATilda) * spParticleSystem->m_qTilda.block_vector9x1(j) + spParticleSystem->m_t;
//                    }
//                }
//            }
//            //spParticleSystem->m_currentVelocities = spParticleSystem->m_previousVelocities*0.1f + UiSettings::stiffness() *  (spParticleSystem->m_g - spParticleSystem->m_previousPositions) / UiSettings::timeStep() + UiSettings::timeStep() * spParticleSystem->m_invMassMatrix * spParticleSystem->m_externalForce;
//            //spParticleSystem->m_currentPositions = spParticleSystem->m_previousPositions + UiSettings::timeStep() * spParticleSystem->m_currentVelocities;
//            spParticleSystem->m_currentPositions =  spParticleSystem->m_currentPositions + (spParticleSystem->m_g - spParticleSystem->m_currentPositions) * UiSettings::stiffness();
//            spParticleSystem->m_currentVelocities = (spParticleSystem->m_currentPositions - spParticleSystem->m_previousPositions) / UiSettings::timeStep();
            
//            size = particles.size();
//            for(int j = 0; j < size; ++j)
//            {
//                shared_ptr<Particle> spParticle = particles[j];
//                if( spParticle->m_fixed )
//                {
//                    spParticleSystem->m_currentPositions.block_vector(j) = spParticleSystem->m_previousPositions.block_vector(j);
//                    spParticleSystem->m_currentVelocities.block_vector(j).setZero();
//                }
//            }
            
//            spParticleSystem->m_previousVelocities = spParticleSystem->m_currentVelocities;
//            spParticleSystem->m_previousPositions = spParticleSystem->m_currentPositions;
//            spParticleSystem->update();
//            emit updateTool();
//        }
        
//        m_busy = false;

//    } else {

//        if ( !m_running ) {
//            LOG( "Simulation not running..." );
//        }
//        if ( m_paused ) {
//            LOG( "Simulation paused..." );
//        }

//    }
//}
//bool Engine::start( )
//{
//    if ( m_spParticleSystems.size() > 0 && !m_running ) {
//        m_running = true;
//        LOG( "SIMULATION STARTED" );
//        m_ticker.start(TICKS);
//        return true;
//    } else {
//        if ( m_spParticleSystems.size() == 0 ) {
//            LOG( "Empty particle system." );
//        }
//        if ( m_running ) {
//            LOG( "Simulation already running." );
//        }
//        return false;
//    }
//}

//void Engine::stop()
//{
//    LOG( "SIMULATION STOPPED" );
//    m_ticker.stop();
//    m_running = false;
//}
//void Engine::pause()
//{
//    LOG( "SIMULATION PAUSED" );
//    m_ticker.stop();
//    m_paused = true;
//}
//void Engine::resume()
//{
//    LOG( "SIMULATION RESUMED" );
//    if ( m_paused ) {
//        m_paused = false;
//        if ( m_running ) m_ticker.start(TICKS);
//    }
//}
//void Engine::reset()
//{
//    if ( !m_running ) {
//        resetParticleSystems();
//        m_time = 0.f;
//    }
//}
//void Engine::resetParticleSystems()
//{
//    for(int i = 0; i < m_spParticleSystems.size(); ++i)
//    {
//        shared_ptr<ParticleSystem>& spParticleSystem = m_spParticleSystems[i];
//        spParticleSystem->m_currentVelocities.setZero();
//        //qDebug() << spParticleSystem->m_currentVelocities.block_vector(0).y();
//        spParticleSystem->m_currentPositions = spParticleSystem->m_originalPositions;
//        QMap<int, shared_ptr<Particle>>& particles = spParticleSystem->getParticles();
//        for(int j = 0; j < particles.size(); ++j)
//        {
//            shared_ptr<Particle> spParticle = particles[j];
//            spParticle->m_fixed = false;
//        }
//        spParticleSystem->m_previousPositions = spParticleSystem->m_currentPositions;
//        spParticleSystem->update();
//    }
//}


/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkIterateNeighborhoodOptimizer.cxx,v $
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "itkIterateNeighborhoodOptimizer.h"
#include "itkCommand.h"
#include "itkEventObject.h"
#include "itkMacro.h"

namespace itk
{

/**
 * Constructor
 */
IterateNeighborhoodOptimizer
::IterateNeighborhoodOptimizer()
{
  m_Stop = false;
  m_Maximize = false;
  m_FullyConnected = true;
  m_CurrentIteration = 0;
  m_CurrentValue = 0.0;
}

/**
 * PrintSelf
 */
void
IterateNeighborhoodOptimizer
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf( os,indent );
  os << indent << "Maximize: " << m_Maximize << std::endl;
  os << indent << "FullyConnected: " << m_FullyConnected << std::endl;
  os << indent << "CurrentIteration: " << m_CurrentIteration;
  os << indent << "CurrentValue: " << m_CurrentValue;
  if ( m_CostFunction )
    {
    os << indent << "CostFunction: " << m_CostFunction;
    }
}


/**
 * Start the optimization
 */
void
IterateNeighborhoodOptimizer
::StartOptimization( void )
{
  m_CurrentIteration   = 0;
  this->SetCurrentPosition( this->GetInitialPosition() );
  this->ResumeOptimization();
}

/**
 * Resume the optimization
 */
void
IterateNeighborhoodOptimizer
::ResumeOptimization( void )
{
  m_Stop = false;

  InvokeEvent( StartEvent() );
  while( !m_Stop )
    {
    try
      {
      m_CurrentValue =
        m_CostFunction->GetValue( this->GetCurrentPosition() );
      }
    catch( ExceptionObject& err )
      {
      // An exception has occurred, terminate immediately.
      StopOptimization();

      // Pass exception to caller
      throw err;
      }

    if( m_Stop )
      {
      break;
      }

    AdvanceOneStep();

    m_CurrentIteration++;
    }
}


/**
 * Stop optimization
 */
void
IterateNeighborhoodOptimizer
::StopOptimization( void )
{
  m_Stop = true;
  InvokeEvent( EndEvent() );
}


/**
 * Advance one Step by searching the neighborhood
 */
void
IterateNeighborhoodOptimizer
::AdvanceOneStep( void )
{
  const unsigned int spaceDimension = m_CostFunction->GetNumberOfParameters();
  const ParametersType & currentPosition = this->GetCurrentPosition();
  ParametersType newPosition( spaceDimension );
  double bestValue = m_CurrentValue;

  if ( !m_FullyConnected )
    {
    // Iterate face connected values
    for(unsigned int j=0; j<spaceDimension; j+=1)
      {
      for(int i=-1; i<=1; i+=2)
        {
        // Get the neighborhood position
        ParametersType neighborPosition( currentPosition );
        neighborPosition[j] += i * m_NeighborhoodSize[j];

        // Check if this value is better than current
        double neighborValue = m_CostFunction->GetValue( neighborPosition );
        if ( m_Maximize && neighborValue > bestValue )
          {
          bestValue = neighborValue;
          newPosition = neighborPosition;
          }
        else if ( !m_Maximize && neighborValue < bestValue )
          {
          bestValue = neighborValue;
          newPosition = neighborPosition;
          }
        }
      }
    }
  else
    {
    // Iterate face+edge+vertex connected values
    for(int i=-1; i<=1; i+=1)
      {
      for(int j=-1; j<=1; j+=1)
        {
        if ( spaceDimension == 2 )
          {
          // Get the neighborhood position
          ParametersType neighborPosition( currentPosition );
          neighborPosition[0] += i * m_NeighborhoodSize[0];
          neighborPosition[1] += j * m_NeighborhoodSize[1];

          // Check if this value is better than current
          double neighborValue = m_CostFunction->GetValue( neighborPosition );
          if ( m_Maximize && neighborValue > bestValue )
            {
            bestValue = neighborValue;
            newPosition = neighborPosition;
            }
          else if ( !m_Maximize && neighborValue < bestValue )
            {
            bestValue = neighborValue;
            newPosition = neighborPosition;
            }
          }// end spaceDimension == 2
        else if ( spaceDimension == 3 )
          {
          for(int k=-1; k<=1; k+=1)
            {
            // Get the neighborhood position
            ParametersType neighborPosition( currentPosition );
            neighborPosition[0] += i * m_NeighborhoodSize[0];
            neighborPosition[1] += j * m_NeighborhoodSize[1];
            neighborPosition[2] += k * m_NeighborhoodSize[2];

            // Check if this value is better than current
            double neighborValue = m_CostFunction->GetValue( neighborPosition );
            if ( m_Maximize && neighborValue > bestValue )
              {
              bestValue = neighborValue;
              newPosition = neighborPosition;
              }
            else if ( !m_Maximize && neighborValue < bestValue )
              {
              bestValue = neighborValue;
              newPosition = neighborPosition;
              }
            }// end for k
          }//end spaceDimension == 3
        }// end for j
      }// end for i
    }// end m_FullyConnected

  if (bestValue == m_CurrentValue)
    {
    // We have found a local maxima/minima
    this->StopOptimization( );
    }
  else
    {
    m_CurrentValue = bestValue;
    this->SetCurrentPosition( newPosition );
    this->InvokeEvent( IterationEvent() );
    }
}

} // end namespace itk
